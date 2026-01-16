#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "globals.hh"

class DetectorConstruction;  // forward declaration
class G4VPhysicalVolume;
class G4VSolid;
class G4Event;
class G4Box;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction(const DetectorConstruction* det);
    ~PrimaryGeneratorAction();


    virtual void GeneratePrimaries(G4Event*);

private:
    G4ParticleGun* fParticleGun;
    const DetectorConstruction* fDet;

    static constexpr double     pi  = 3.14159265358979323846;


    G4bool ComputeMinimalConeForBox(const G4ThreeVector& apexWorld, const G4ThreeVector& boxPos, G4ThreeVector boxHalfs,
        G4ThreeVector& axisOut, G4double& thetaOut) const;


    // Helpers
    static G4bool PointInOBB(const G4ThreeVector& P,
        const G4ThreeVector& C, const G4ThreeVector& half,
        const G4ThreeVector& ex, const G4ThreeVector& ey, const G4ThreeVector& ez);

    static void BoxVertices(const G4ThreeVector& C, const G4ThreeVector& half,
        const G4ThreeVector& ex, const G4ThreeVector& ey, const G4ThreeVector& ez,
        std::array<G4ThreeVector,8>& V);

    G4ThreeVector SampleDirectionInCone(const G4ThreeVector& axis_unit,
            G4double alpha);

    G4ThreeVector      fApex{0,0,0};
    G4VPhysicalVolume* fTargetBoxPV{nullptr};
};



#endif
