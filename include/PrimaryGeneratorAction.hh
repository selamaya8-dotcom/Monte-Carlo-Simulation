#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4ThreeVector.hh" // Added for G4ThreeVector
#include "globals.hh"

class DetectorConstruction;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
  public:
    PrimaryGeneratorAction(const DetectorConstruction* det);
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* event) override;

  private:
    G4ParticleGun* fParticleGun;
    const DetectorConstruction* fDet;

    G4bool DoesRayHitBox(G4ThreeVector origin, G4ThreeVector dir,
                         G4ThreeVector boxCenter, G4ThreeVector boxHalf);
};

#endif
