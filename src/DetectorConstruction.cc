#include "DetectorConstruction.hh"
#include "MuonDetectorA.hh"
#include "MuonDetectorB.hh"

#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SDManager.hh"
#include "G4GenericMessenger.hh"

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(), 
  fSoilDensity(2.0*g/cm3), fDetHx(0), fDetHy(0), fDetHz(0),
  fBuildHx(0), fBuildHy(0), fBuildHz(0)
{
    // Define UI command for soil density
    fMessenger = new G4GenericMessenger(this, "/soil/", "Soil density control");
    fMessenger->DeclarePropertyWithUnit("density", "g/cm3", fSoilDensity, "Soil density in g/cm3");
}

DetectorConstruction::~DetectorConstruction() {
    delete fMessenger;
}

void DetectorConstruction::DefineMaterials() {
    auto nist = G4NistManager::Instance();

    // Define Elements for Soil
    G4Element* elSi = nist->FindOrBuildElement("Si");
    G4Element* elMg = nist->FindOrBuildElement("Mg");
    G4Element* elAl = nist->FindOrBuildElement("Al");
    G4Element* elCa = nist->FindOrBuildElement("Ca");
    G4Element* elFe = nist->FindOrBuildElement("Fe");
    G4Element* elNa = nist->FindOrBuildElement("Na");
    G4Element* elTi = nist->FindOrBuildElement("Ti");
    G4Element* elO  = nist->FindOrBuildElement("O");

    // Define Component Oxides
    G4Material* SiO2 = new G4Material("SiO2", 2.65*g/cm3, 2);
    SiO2->AddElement(elSi, 1); SiO2->AddElement(elO, 2);

    G4Material* MgO = new G4Material("MgO", 3.58*g/cm3, 2);
    MgO->AddElement(elMg, 1); MgO->AddElement(elO, 1);

    G4Material* Al2O3 = new G4Material("Al2O3", 3.95*g/cm3, 2);
    Al2O3->AddElement(elAl, 2); Al2O3->AddElement(elO, 3);

    G4Material* CaO = new G4Material("CaO", 3.34*g/cm3, 2);
    CaO->AddElement(elCa, 1); CaO->AddElement(elO, 1);

    G4Material* FeO = new G4Material("FeO", 5.70*g/cm3, 2);
    FeO->AddElement(elFe, 1); FeO->AddElement(elO, 1);

    G4Material* Na2O = new G4Material("Na2O", 2.27*g/cm3, 2);
    Na2O->AddElement(elNa, 2); Na2O->AddElement(elO, 1);

    G4Material* TiO2 = new G4Material("TiO2", 4.23*g/cm3, 2);
    TiO2->AddElement(elTi, 1); TiO2->AddElement(elO, 2);

    // Create Soil Mixture
    G4Material* Soil = new G4Material("Soil", fSoilDensity, 7);
    Soil->AddMaterial(SiO2, 45.0*perCent);
    Soil->AddMaterial(MgO, 35.0*perCent);
    Soil->AddMaterial(Al2O3, 7.0*perCent);
    Soil->AddMaterial(CaO, 7.0*perCent);
    Soil->AddMaterial(FeO, 5.0*perCent);
    Soil->AddMaterial(Na2O, 0.7*perCent);
    Soil->AddMaterial(TiO2, 0.3*perCent);

    // Define Cardboard (Cellulose)
    G4Material* cardboard = new G4Material("Cardboard", 0.7*g/cm3, 3);
    cardboard->AddElement(nist->FindOrBuildElement("C"), 6);
    cardboard->AddElement(nist->FindOrBuildElement("H"), 10);
    cardboard->AddElement(nist->FindOrBuildElement("O"), 5);
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    return ConstructVolumes();
}

G4VPhysicalVolume* DetectorConstruction::ConstructVolumes() {
    DefineMaterials();
    auto nist = G4NistManager::Instance();
    auto air  = nist->FindOrBuildMaterial("G4_AIR");
    auto soil = G4Material::GetMaterial("Soil");

    // 1. World
    auto worldS = new G4Box("World", 50*m, 50*m, 50*m);
    auto worldLV = new G4LogicalVolume(worldS, air, "World");
    auto worldPV = new G4PVPlacement(nullptr, {}, worldLV, "World", nullptr, false, 0);

    // 2. Detector Enclosure (Cardboard Box)
    G4double boxXY = 15*cm;
    G4double boxZ  = 5*cm;
    fDetCenter = G4ThreeVector(0, (boxZ/2.0) + 0.5*m, 0);
    
    // Member variables for the Primary Generator's hit-check
    fDetHx = boxXY/2.0; fDetHy = boxZ/2.0; fDetHz = boxXY/2.0;

    auto outerBox = new G4Box("OuterBox", boxXY/2, boxXY/2, boxZ/2);
    auto innerBox = new G4Box("InnerBox", boxXY/2 - 2*cm, boxXY/2 - 2*cm, boxZ/2 - 2*cm);
    auto detS     = new G4SubtractionSolid("BoxShell", outerBox, innerBox);
    auto detLV    = new G4LogicalVolume(detS, G4Material::GetMaterial("Cardboard"), "BoxLV");

    G4RotationMatrix* rotX90 = new G4RotationMatrix();
    rotX90->rotateX(90.*deg);
    new G4PVPlacement(rotX90, fDetCenter, detLV, "BoxPV", worldLV, false, 0);

    // 3. Sensitive Scintillator Plates
    G4double detXY = 10*cm, detZ = 2*cm;
    auto detS_Plate = new G4Box("ScintPlate", detXY/2, detXY/2, detZ/2);
    auto scintMat   = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

    det1LV = new G4LogicalVolume(detS_Plate, scintMat, "Det1LV");
    det2LV = new G4LogicalVolume(detS_Plate, scintMat, "Det2LV");

    G4double padding = 1*cm;
    new G4PVPlacement(nullptr, {0, 0,  boxZ/2 - padding - detZ/2}, det1LV, "Det1PV", detLV, false, 0);
    new G4PVPlacement(nullptr, {0, 0, -boxZ/2 + padding + detZ/2}, det2LV, "Det2PV", detLV, false, 0);

    // 4. Target Volume (Soil Cube or Sphere)
    G4double cubeHalf = 0.5 * m;
    G4double cubeY    = fDetCenter.y() + (boxZ/2) + 1.0*m + cubeHalf;
    fBuildCenter      = G4ThreeVector(0, cubeY, 0);
    fBuildHx = fBuildHy = fBuildHz = cubeHalf;

    /* --- OPTION: Sphere Target ---
    G4double sphereRadius = 0.5 * m;
    auto targetS = new G4Sphere("Target", 0, sphereRadius, 0, 360*deg, 0, 180*deg);
    fBuildHx = fBuildHy = fBuildHz = sphereRadius;
    ------------------------------ */

    auto targetS  = new G4Box("SoilTarget", cubeHalf, cubeHalf, cubeHalf);
    auto targetLV = new G4LogicalVolume(targetS, soil, "TargetLV");
    new G4PVPlacement(nullptr, fBuildCenter, targetLV, "TargetPV", worldLV, false, 0);

    return worldPV;
}

void DetectorConstruction::ConstructSDandField() {
    auto sdManager = G4SDManager::GetSDMpointer();

    auto muonSD_A = new MuonDetectorA("MuonSD_A");
    sdManager->AddNewDetector(muonSD_A);
    det1LV->SetSensitiveDetector(muonSD_A);

    auto muonSD_B = new MuonDetectorB("MuonSD_B");
    sdManager->AddNewDetector(muonSD_B);
    det2LV->SetSensitiveDetector(muonSD_B);
}
