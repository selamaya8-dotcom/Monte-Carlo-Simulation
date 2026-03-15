#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* det)
: fDet(det)
{
    fParticleGun = new G4ParticleGun(1);
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete fParticleGun;
}

G4double PrimaryGeneratorAction::GetMuonEnergy(G4double theta) {
    G4double E;
    G4double cosTheta = std::cos(theta);
    // Increased max probability for safer rejection sampling
    G4double maxProb = 0.14;

    while (true) {
        G4double minE = 1.0 * GeV;
        G4double maxE = 100.0 * GeV;
        E = minE + G4UniformRand() * (maxE - minE);

        G4double E_GeV = E / GeV;
        G4double term1 = 1.0 / (1.0 + (1.1 * E_GeV * cosTheta / 115.0));
        G4double term2 = 0.054 / (1.0 + (1.1 * E_GeV * cosTheta / 850.0));
        G4double probability = std::pow(E_GeV, -2.7) * (term1 + term2);

        if (G4UniformRand() * maxProb < probability) {
            return E;
        }
    }
}

// Improved Intersection Check: Standard Ray-Sphere Algebra
// Helper function: Slab method for Ray-Box intersection
G4bool PrimaryGeneratorAction::DoesRayHitBox(G4ThreeVector origin, G4ThreeVector dir,
                                             G4ThreeVector boxCenter, G4ThreeVector boxHalf) {
    // Translate origin to box-local coordinates
    G4ThreeVector relOrigin = origin - boxCenter;

    G4double tMin = -kInfinity;
    G4double tMax = kInfinity;

    // Check intersection with each pair of planes (X, Y, Z)
    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < 1e-9) { // Ray is parallel to planes
            if (std::abs(relOrigin[i]) > boxHalf[i]) return false;
        } else {
            G4double t1 = (-boxHalf[i] - relOrigin[i]) / dir[i];
            G4double t2 = (boxHalf[i] - relOrigin[i]) / dir[i];

            tMin = std::max(tMin, std::min(t1, t2));
            tMax = std::min(tMax, std::max(t1, t2));
        }
    }
    // Ray hits if the intersection interval is valid and in front of the ray
    return tMax >= tMin && tMax > 0;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    G4ThreeVector detCenter = fDet->GetDetCenter();
    G4ThreeVector boxHalf = fDet->GetBuildingHalfSize();
    G4double generationHeight = 3.0 * m;

    G4ThreeVector sourcePos;
    G4ThreeVector direction;
    G4double theta;
    G4bool hitsStructure = false;

    // 1. Re-generation loop: Keep trying until a valid muon hits the box
    while (!hitsStructure) {
        // Sample Position on the plate
        G4double xPos = detCenter.x() + (G4UniformRand() - 0.5) * 2.3 * boxHalf.x();
        G4double zPos = detCenter.z() + (G4UniformRand() - 0.5) * 2.3 * boxHalf.z();
        sourcePos.set(xPos, generationHeight, zPos);

        // Sample Angle (cos^2 distribution) with 70 deg limit
        G4double maxAngle = 75.0 * deg;
        do {
            theta = std::acos(std::pow(G4UniformRand(), 1.0/3.0));
        } while (theta > maxAngle);

        G4double phi = 2.0 * M_PI * G4UniformRand();
        direction.set(
            std::sin(theta) * std::cos(phi),
            -std::cos(theta),
            std::sin(theta) * std::sin(phi)
        );

        // 2. Check if this specific ray actually intersects the building
        if (DoesRayHitBox(sourcePos, direction, detCenter, boxHalf)) {
            hitsStructure = true;
        }
    }

    // 3. Set properties once a valid ray is found
    G4double energy = GetMuonEnergy(theta);
    fParticleGun->SetParticlePosition(sourcePos);
    fParticleGun->SetParticleMomentumDirection(direction);
    fParticleGun->SetParticleEnergy(energy);

    // Pass to EventAction
    auto eventAction = (EventAction*)G4RunManager::GetRunManager()->GetUserEventAction();
    if (eventAction) {
        eventAction->SetGenAngle(theta);
        eventAction->SetGenEnergy(energy);
        eventAction->SetGenPosition(sourcePos);
    }

    fParticleGun->GeneratePrimaryVertex(event);
}
