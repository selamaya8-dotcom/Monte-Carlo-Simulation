//with material
#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4MuonMinus.hh"
#include "G4Event.hh"
#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4AffineTransform.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>

namespace {
    inline G4ThreeVector Normalize(const G4ThreeVector& v) {
      G4double n = v.mag();
      return (n > 0) ? v/n : v;
    }
    inline G4double Clamp(G4double x, G4double lo=-1.0, G4double hi=1.0){
      return std::max(lo, std::min(hi, x));
    }
  }


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* det):
fDet(det)
{
    fParticleGun = new G4ParticleGun(1);

}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete fParticleGun;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4bool DoesRayHitSphere(G4ThreeVector origin, G4ThreeVector dir,
                        G4ThreeVector center, G4double radius) {
    G4ThreeVector L = center - origin;
    G4double tca = L.dot(dir);
    if (tca < 0) return false; // Sphere is behind the ray

    G4double d2 = L.dot(L) - tca * tca;
    if (d2 > radius * radius) return false; // Ray misses the sphere

    return true;
}
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(4.0 * GeV);

    G4ThreeVector detCenter = fDet->GetDetCenter();
    G4ThreeVector detHalf   = fDet->GetDetHalfSize();
    G4ThreeVector buildCenter = fDet->GetBuildingCenter();
    G4double sphereRadius     = fDet->GetBuildingHalfSize().x(); // Assuming radius is stored here

    G4ThreeVector direction;
    G4ThreeVector sourcePos;
    G4bool hitsBoth = false;
    G4int trials = 0;

    while(!hitsBoth && trials < 10000) {
        trials++;

        // 1. Sample the cosmic angular distribution
        G4double theta = std::acos(std::pow(G4UniformRand(), 1.0/3.0));
        G4double phi = 2.0 * M_PI * G4UniformRand();

        direction.set(
            std::sin(theta) * std::cos(phi),
            -std::cos(theta),
            std::sin(theta) * std::sin(phi)
        );

        // 2. Pick a point on the DETECTOR
        G4double targetX = (G4UniformRand() - 0.5) * 2.0 * detHalf.x() + detCenter.x();
        G4double targetZ = (G4UniformRand() - 0.5) * 2.0 * detHalf.z() + detCenter.z();
        G4double targetY = detCenter.y();
        G4ThreeVector targetPoint(targetX, targetY, targetZ);

        // 3. Backtrack to source height (Y=5m)
        G4double startY = 3.0 * m;
        G4double deltaY = std::abs(startY - targetY);
        sourcePos = targetPoint - direction * (deltaY / std::abs(direction.y()));

        // 4. Precise Sphere Intersection Check
        if (DoesRayHitSphere(sourcePos, direction, buildCenter, sphereRadius)) {
            hitsBoth = true;
        }
    }

    fParticleGun->SetParticlePosition(sourcePos);
    fParticleGun->SetParticleMomentumDirection(direction);
    fParticleGun->GeneratePrimaryVertex(event);
}
/*
//muons stright at the detector
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {

    // 1. Define the particle
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(4.0 * GeV);

    // 2. Retrieve geometry data from DetectorConstruction
    G4ThreeVector detCenter = fDet->GetDetCenter();   // center of first detector
    G4ThreeVector detHalf   = fDet->GetDetHalfSize(); // half-size (x,y,z)

    // 3. Define a random spread in x and y around the detector center
    G4double spread = 5 * cm;  // total spread range; small beam
    G4double dx = (CLHEP::HepRandom::getTheEngine()->flat() - 0.5) * 2.0 * spread;
    G4double dy = (CLHEP::HepRandom::getTheEngine()->flat() - 0.5) * 2.0 * spread;

    // 4. Define the start position slightly in front of the detector (assuming +z goes into detector)
    G4double offset = 5*m;
    G4ThreeVector startPosition = detCenter - G4ThreeVector(0., 0., detHalf.z() + offset);
    startPosition.setX(startPosition.x() + dx);
    startPosition.setY(startPosition.y() + dy);

    // 5. Set a straight direction toward the detector
    G4ThreeVector direction(0., 0., 1.);

    fParticleGun->SetParticlePosition(startPosition);
    fParticleGun->SetParticleMomentumDirection(direction);

    // 6. Generate the event
    fParticleGun->GeneratePrimaryVertex(event);
}
*/
