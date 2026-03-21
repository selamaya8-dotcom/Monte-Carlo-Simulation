#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4GenericMessenger;

/**
 * @brief Constructs the world, a soil target (cube or sphere), and a 
 * plastic scintillator detector encased in a cardboard box.
 */
class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct() override;
    virtual void ConstructSDandField() override;

    // Accessors for PrimaryGeneratorAction to ensure muons are aimed correctly
    const G4ThreeVector& GetDetCenter() const { return fDetCenter; }
    G4ThreeVector GetBuildingHalfSize() const { return {fBuildHx, fBuildHy, fBuildHz}; }
    
    // Setters
    void SetSoilDensity(G4double density) { fSoilDensity = density; }

private:
    void DefineMaterials();
    G4VPhysicalVolume* ConstructVolumes();

    // Messenger for UI commands
    G4GenericMessenger* fMessenger;

    // Logical Volumes for SD assignment
    G4LogicalVolume* det1LV = nullptr;
    G4LogicalVolume* det2LV = nullptr;

    // Geometry parameters
    G4double fSoilDensity;
    G4double fDetHx, fDetHy, fDetHz;
    G4ThreeVector fDetCenter;
    
    G4double fBuildHx, fBuildHy, fBuildHz;
    G4ThreeVector fBuildCenter;
};

#endif
