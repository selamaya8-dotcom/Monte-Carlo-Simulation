#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"



class G4LogicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    DetectorConstruction();
    ~DetectorConstruction();


    virtual G4VPhysicalVolume* Construct() override;

    virtual void ConstructSDandField() override;


    const G4ThreeVector& GetDetCenter() const { return fDetCenter; }
    G4ThreeVector GetDetHalfSize() const { return {fDetHx, fDetHy, fDetHz}; }
    G4double GetWorldXHalfLength() const { return fWorldHx; }
    G4ThreeVector GetBuildingHalfSize() const { return {fBuildHx, fBuildHy, fBuildHz}; }
    const G4ThreeVector& GetBuildingCenter() const { return fBuildCenter; }

    void SetSoilDensity(G4double density) { fSoilDensity = density; }

private:
    G4double fSoilDensity = 0.2*g/cm3;
    G4GenericMessenger* fMessenger;
    G4LogicalVolume* fDetectorLogic = nullptr;
    G4LogicalVolume* det1LV = nullptr;
    G4LogicalVolume* det2LV = nullptr;


    void DefineMaterials();
    G4VPhysicalVolume* ConstructVolumes();

    G4double fDetHx=0, fDetHy=0, fDetHz=0;
    G4ThreeVector fDetCenter = {0,0,0};
    G4double fBuildHx=0, fBuildHy=0, fBuildHz=0;
    G4ThreeVector fBuildCenter = {0,0,0};

    G4VPhysicalVolume* fBuildingPV = nullptr;

    G4double fWorldHx = 0;
    G4double fSkyZ = 0;
};


#endif
