#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* det)
: fDet(det) {
    // Initialize particle gun with 1 particle per shot
    fParticleGun = new G4ParticleGun(1);
    
    // Default to Muons (mu-)
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete fParticleGun;
}

/**
 * @brief Uses rejection sampling to pick an energy from the cosmic muon spectrum.
 * Based on the modified Gaisser formula for sea-level intensity.
 */
G4double PrimaryGeneratorAction::GetMuonEnergy(G4double theta) {
    G4double E;
    G4double cosTheta = std::cos(theta);
    const G4double maxProb = 0.14; // Upper bound for the PDF

    while (true) {
        G4double minE = 1.0 * GeV;
        G4double maxE = 100.0 * GeV;
        E = minE + G4UniformRand() * (maxE - minE);

        G4double E_GeV = E / GeV;
        
        // Simplified Gaisser-style parametrization
        G4double term1 = 1.0 / (1.0 + (1.1 * E_GeV * cosTheta / 115.0));
        G4double term2 = 0.054 / (1.0 + (1.1 * E_GeV * cosTheta / 850.0));
        G4double probability = std::pow(E_GeV, -2.7) * (term1 + term2);

        if (G4UniformRand() * maxProb < probability) {
            return E;
        }
    }
}

/**
 * @brief Ray-Box intersection check (Slab Method).
 * Ensures we only simulate particles that will actually interact with our building.
 */
G4bool PrimaryGeneratorAction::DoesRayHitBox(G4ThreeVector origin, G4ThreeVector dir,
                                             G4ThreeVector boxCenter, G4ThreeVector boxHalf) {
    G4ThreeVector relOrigin = origin - boxCenter;
    G4double tMin = -kInfinity;
    G4double tMax = kInfinity;

    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < 1e-9) {
            if (std::abs(relOrigin[i]) > boxHalf[i]) return false;
        } else {
            G4double t1 = (-boxHalf[i] - relOrigin[i]) / dir[i];
            G4double t2 = (boxHalf[i] - relOrigin[i]) / dir[i];
            tMin = std::max(tMin, std::min(t1, t2));
            tMax = std::min(tMax, std::max(t1, t2));
        }
    }
    return tMax >= tMin && tMax > 0;
}

/**
 * @brief Generates the primary vertex. 
 * Samples positions and angles until a valid "hit" trajectory is found.
 */
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    G4ThreeVector detCenter = fDet->GetDetCenter();
    G4ThreeVector boxHalf = fDet->GetBuildingHalfSize();
    G4double generationHeight = 3.0 * m; // Height of the virtual emission plane

    G4ThreeVector sourcePos;
    G4ThreeVector direction;
    G4double theta;
    G4bool hitsStructure = false;

    // Loop until we find a trajectory that hits the target volume
    while (!hitsStructure) {
        // 1. Sample position on a horizontal plane above the detector
        G4double xPos = detCenter.x() + (G4UniformRand() - 0.5) * 2.3 * boxHalf.x();
        G4double zPos = detCenter.z() + (G4UniformRand() - 0.5) * 2.3 * boxHalf.z();
        sourcePos.set(xPos, generationHeight, zPos);

        // 2. Sample zenith angle (cos^2 distribution for muons)
        G4double maxAngle = 75.0 * deg;
        do {
            theta = std::acos(std::pow(G4UniformRand(), 1.0/3.0));
        } while (theta > maxAngle);

        // 3. Sample azimuthal angle and set direction
        G4double phi = 2.0 * M_PI * G4UniformRand();
        direction.set(std::sin(theta) * std::cos(phi), -std::cos(theta), std::sin(theta) * std::sin(phi));

        // 4. Efficiency Check
        if (DoesRayHitBox(sourcePos, direction, detCenter, boxHalf)) {
            hitsStructure = true;
        }
    }

    // Assign final energy and gun parameters
    G4double energy = GetMuonEnergy(theta);
    fParticleGun->SetParticlePosition(sourcePos);
    fParticleGun->SetParticleMomentumDirection(direction);
    fParticleGun->SetParticleEnergy(energy);

    // Data transfer to EventAction for analysis
    auto eventAction = static_cast<EventAction*>(G4RunManager::GetRunManager()->GetUserEventAction());
    if (eventAction) {
        eventAction->SetGenAngle(theta);
        eventAction->SetGenEnergy(energy);
        eventAction->SetGenPosition(sourcePos);
    }

    fParticleGun->GeneratePrimaryVertex(event);
}
