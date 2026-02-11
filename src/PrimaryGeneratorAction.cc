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
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    // --- 1. Setup Particle ---
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(4.0 * GeV);

    // --- 2. Define Geometry Targets ---
    G4ThreeVector detPos = fDet->GetDetCenter();
    G4ThreeVector bldPos = fDet->GetBuildingCenter();

    // We want the source to be placed "up and to the side"
    // to simulate slanted cosmic rays hitting the building side.
    G4ThreeVector offset(0*m, 5.0*m, -5*m); // Source is 20m up, 15m to the side
    G4ThreeVector sourceCenter = bldPos + offset;

    // --- 3. Generate Position on a Disc (The "Window") ---
    // This makes the source diffuse rather than a single point
    CLHEP::HepRandomEngine* engine = CLHEP::HepRandom::getTheEngine();
    G4double radius = 5.0 * m; // Size of the emitting "cloud"
    G4double r = radius * std::sqrt(engine->flat());
    G4double phiPos = 2.0 * CLHEP::pi * engine->flat();

    // Position the particle on a horizontal disc at sourceCenter
    G4ThreeVector startPos = sourceCenter + G4ThreeVector(r * std::cos(phiPos), 0, r * std::sin(phiPos));
    fParticleGun->SetParticlePosition(startPos);

    // --- 4. Biased Directional Sampling ---
    // Aim the "peak" of the cos^2 distribution toward the detector
    G4ThreeVector mainAxis = (detPos - startPos).unit();

    G4double cosTheta;
    G4bool accepted = false;
    while (!accepted) {
        // We limit sampling to a cone (e.g., within 30 degrees of the target)
        // to prevent muons from firing backwards or away from the setup.
        G4double minCos = std::cos(30.0 * deg);
        G4double x = minCos + (1.0 - minCos) * engine->flat(); // Sample cos(theta)

        G4double y = engine->flat();
        if (y < (x * x)) { // cos^2(theta) weighting
            cosTheta = x;
            accepted = true;
        }
    }

    G4double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
    G4double phiDir = 2.0 * CLHEP::pi * engine->flat();

    // Orthonormal basis for the specific aim
    G4ThreeVector k = mainAxis;
    G4ThreeVector i = (std::fabs(k.z()) < 0.999) ? k.cross(G4ThreeVector(0,0,1)).unit()
                                                : k.cross(G4ThreeVector(1,0,0)).unit();
    G4ThreeVector j = k.cross(i);

    G4ThreeVector direction = i*(sinTheta * std::cos(phiDir)) +
                              j*(sinTheta * std::sin(phiDir)) +
                              k*cosTheta;

    fParticleGun->SetParticleMomentumDirection(direction.unit());
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
