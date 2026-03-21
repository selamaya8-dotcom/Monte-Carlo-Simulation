#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class G4ParticleGun;
class G4Event;
class DetectorConstruction;

/**
 * @brief Generates primary muons with energy and angular distributions 
 * representative of sea-level cosmic rays.
 */
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
  public:
    PrimaryGeneratorAction(const DetectorConstruction* det);
    virtual ~PrimaryGeneratorAction();

    // Main method called at the beginning of every event
    virtual void GeneratePrimaries(G4Event* event) override;

  private:
    G4ParticleGun* fParticleGun;
    const DetectorConstruction* fDet;

    // Calculates muon energy based on the Smith-Duller / Gaisser distribution
    G4double GetMuonEnergy(G4double theta);

    // Determines if a generated muon trajectory will actually strike the detector geometry
    G4bool DoesRayHitBox(G4ThreeVector origin, G4ThreeVector dir,
                         G4ThreeVector boxCenter, G4ThreeVector boxHalf);
};

#endif
