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
G4bool RayIntersectsSphere(G4ThreeVector origin, G4ThreeVector dir,
                           G4ThreeVector center, G4double radius) {
    G4ThreeVector oc = origin - center;
    G4double b = oc.dot(dir);
    G4double c = oc.dot(oc) - radius * radius;
    G4double discriminant = b*b - c;

    if (discriminant < 0) return false;

    // Check if the intersection is in front of the ray
    G4double t = -b - std::sqrt(discriminant);
    if (t < 0) t = -b + std::sqrt(discriminant);

    return t >= 0;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    G4ThreeVector detCenter = fDet->GetDetCenter();
    G4double sphereRadius   = fDet->GetBuildingHalfSize().x();

    // The height you requested
    G4double generationHeight = 3.0 * m;

    G4ThreeVector direction;
    G4ThreeVector sourcePos;
    G4double theta;
    G4bool hitsTarget = false;

    while(!hitsTarget) {
        // 1. Sample Angle
        theta = std::acos(std::pow(G4UniformRand(), 1.0/3.0));
        G4double phi = 2.0 * M_PI * G4UniformRand();

        direction.set(
            std::sin(theta) * std::cos(phi),
            -std::cos(theta),
            std::sin(theta) * std::sin(phi)
        );

        G4double projectionSpread = sphereRadius;

        G4double xOffset = (G4UniformRand() - 0.5) * 2.0 * projectionSpread;
        G4double zOffset = (G4UniformRand() - 0.5) * 2.0 * projectionSpread;

        sourcePos.set(detCenter.x() + xOffset, generationHeight, detCenter.z() + zOffset);

        // 3. Mathematical Check
        if (RayIntersectsSphere(sourcePos, direction, detCenter, sphereRadius)) {
            hitsTarget = true;
        }
    }

    G4double energy = GetMuonEnergy(theta);

    // 2. Set the gun properties
    fParticleGun->SetParticleEnergy(energy);
    fParticleGun->SetParticlePosition(sourcePos);
    fParticleGun->SetParticleMomentumDirection(direction);

    // 3. Pass both values to EventAction
    auto eventAction = (EventAction*)G4RunManager::GetRunManager()->GetUserEventAction();
    if (eventAction) {
        eventAction->SetGenAngle(theta);
        eventAction->SetGenEnergy(energy); // Now 'energy' is defined!
    }

    fParticleGun->GeneratePrimaryVertex(event);
}
