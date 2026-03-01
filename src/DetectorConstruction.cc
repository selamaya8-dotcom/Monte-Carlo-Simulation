#include "DetectorConstruction.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "MuonDetectorA.hh"
#include "MuonDetectorB.hh"
#include "G4SubtractionSolid.hh"
#include "G4SDManager.hh"
#include "G4GenericMessenger.hh"
#include "G4Sphere.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
:G4VUserDetectorConstruction(), fDetectorLogic(0) , det1LV(0), det2LV(0), fSoilDensity(0.*g/cm3)
{
  fMessenger = new G4GenericMessenger(this, "/soil/", "Soil density control");
  fMessenger->DeclarePropertyWithUnit("density", "g/cm3", fSoilDensity, "Soil density in g/cm3");

  G4cout << "DENSITY!!!!   =" << fSoilDensity/(g/cm3)<< G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  return ConstructVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::ConstructVolumes()
{
    DefineMaterials();
    auto nist = G4NistManager::Instance();

    // 1. Define Materials
    G4Element* elSi = new G4Element("Silicon", "Si", 14., 28.0855*g/mole);
    G4Element* elMg = new G4Element("Magnesium", "Mg", 12., 24.305*g/mole);
    G4Element* elAl = new G4Element("Aluminum", "Al", 13., 26.9815*g/mole);
    G4Element* elCa = new G4Element("Calcium", "Ca", 20., 40.078*g/mole);
    G4Element* elFe = new G4Element("Iron", "Fe", 26., 55.845*g/mole);
    G4Element* elNa = new G4Element("Sodium", "Na", 11., 22.9897*g/mole);
    G4Element* elTi = new G4Element("Titanium", "Ti", 22., 47.867*g/mole);
    G4Element* elO  = new G4Element("Oxygen", "O", 8., 16.00*g/mole);

    G4Material* SiO2 = new G4Material("SiliconDioxide", 2.65*g/cm3, 2);
    SiO2->AddElement(elSi, 1); SiO2->AddElement(elO, 2);
    G4Material* MgO = new G4Material("MagnesiumOxide", 3.58*g/cm3, 2);
    MgO->AddElement(elMg, 1); MgO->AddElement(elO, 1);
    G4Material* Al2O3 = new G4Material("AluminumOxide", 3.95*g/cm3, 2);
    Al2O3->AddElement(elAl, 2); Al2O3->AddElement(elO, 3);
    G4Material* CaO = new G4Material("CalciumOxide", 3.34*g/cm3, 2);
    CaO->AddElement(elCa, 1); CaO->AddElement(elO, 1);
    G4Material* FeO = new G4Material("IronOxide", 5.70*g/cm3, 2);
    FeO->AddElement(elFe, 1); FeO->AddElement(elO, 1);
    G4Material* Na2O = new G4Material("SodiumOxide", 2.27*g/cm3, 2);
    Na2O->AddElement(elNa, 2); Na2O->AddElement(elO, 1);
    G4Material* TiO2 = new G4Material("TitaniumDioxide", 4.23*g/cm3, 2);
    TiO2->AddElement(elTi, 1); TiO2->AddElement(elO, 2);

    if (fSoilDensity <= 0) fSoilDensity = 2.0*g/cm3;
    G4Material* Soil = new G4Material("Soil", fSoilDensity, 7);
    Soil->AddMaterial(SiO2, 45.0*perCent);
    Soil->AddMaterial(MgO, 35.0*perCent);
    Soil->AddMaterial(Al2O3, 7.0*perCent);
    Soil->AddMaterial(CaO, 7.0*perCent);
    Soil->AddMaterial(FeO, 5.0*perCent);
    Soil->AddMaterial(Na2O, 0.7*perCent);
    Soil->AddMaterial(TiO2, 0.3*perCent);

    auto air = nist->FindOrBuildMaterial("G4_AIR");

    // 2. World and Ground
    auto worldS = new G4Box("World", 50*m, 50*m, 50*m);
    auto worldLV = new G4LogicalVolume(worldS, air, "World");
    auto worldPV = new G4PVPlacement(nullptr, {}, worldLV, "World", nullptr, false, 0);
    fWorldHx = worldS->GetXHalfLength();

    auto groundS = new G4Box("Ground", 50*m, 50*m, 50*m);
    auto groundLV = new G4LogicalVolume(groundS, Soil, "Ground");
    new G4PVPlacement(nullptr, G4ThreeVector(0, -50*m, 0), groundLV, "Ground", worldLV, false, 0);

    // 3. Setup Detector (Define boxZ and detPlacement FIRST)
    G4double boxXY = 11*cm;
    G4double boxZ = 25*cm; // Now it's declared
    G4ThreeVector detPlacement = G4ThreeVector(0, (boxZ/2) + 0.5*m, 0); // Now it's declared

    fDetHx = boxXY/2;
    fDetHy = boxZ/2;
    fDetHz = boxXY/2;
    fDetCenter = detPlacement;

    auto outerBox = new G4Box("OuterBox", boxXY/2, boxXY/2, boxZ/2);
    auto innerBox = new G4Box("InnerBox", boxXY/2 - 1*cm, boxXY/2 - 1*cm, boxZ/2 - 1*cm);
    auto detS = new G4SubtractionSolid("Shell", outerBox, innerBox, nullptr, G4ThreeVector(0,0,0));

    G4Material* cardboard = nist->FindOrBuildMaterial("G4_WOOD");

    // SAFETY CHECK: If cardboard is null, define it manually so the code doesn't crash
    if (!cardboard) {
        G4cout << "--> Warning: G4_WOOD not found in NIST. Defining Cardboard manually." << G4endl;
        G4Element* elC = nist->FindOrBuildElement("C");
        G4Element* elH = nist->FindOrBuildElement("H");
        G4Element* elO = nist->FindOrBuildElement("O");

        // Cellulose-based cardboard definition (approximate density 0.7 g/cm3)
        cardboard = new G4Material("Cardboard", 0.7*g/cm3, 3);
        cardboard->AddElement(elC, 6);
        cardboard->AddElement(elH, 10);
        cardboard->AddElement(elO, 5);
    }

    // Now assign it to the logical volume
    auto detLV = new G4LogicalVolume(detS, cardboard, "Box");
    G4RotationMatrix* rotVertical = new G4RotationMatrix();
    rotVertical->rotateX(90.*deg);
    new G4PVPlacement(rotVertical, detPlacement, detLV, "Box", worldLV, false, 0);

    G4double detXY = 10*cm, detZ = 2*cm;
    auto det1S = new G4Box("Det1", detXY/2, detXY/2, detZ/2);
    det1LV = new G4LogicalVolume(det1S, nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"), "Det1");
    det2LV = new G4LogicalVolume(det1S, nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"), "Det2");

    G4double wallPadding = 1*cm;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0,  boxZ/2 - wallPadding - detZ/2), det1LV, "Det1", detLV, false, 0);
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, -boxZ/2 + wallPadding + detZ/2), det2LV, "Det2", detLV, false, 0);

    // 4. Setup Sphere (Now we can use detPlacement and boxZ)
    G4double sphereRadius = 0.5 * m;
    G4double sphereY = detPlacement.y() + (boxZ/2) + 1.0*m + sphereRadius;
    G4ThreeVector finalSpherePos = G4ThreeVector(0, sphereY, 0);

    fBuildHx = sphereRadius;
    fBuildHy = sphereRadius;
    fBuildHz = sphereRadius;
    fBuildCenter = finalSpherePos;

    auto concreteSphereSolid = new G4Sphere("ConcreteSphere", 0.*cm, sphereRadius, 0.*deg, 360.*deg, 0.*deg, 180.*deg);
    auto concreteSphereLogical = new G4LogicalVolume(concreteSphereSolid, Soil, "ConcreteSphereLV");
    new G4PVPlacement(nullptr, finalSpherePos, concreteSphereLogical, "ConcreteSphere", worldLV, false, 0);

    return worldPV;
}

void DetectorConstruction::ConstructSDandField() {
    // Create separate sensitive detectors for each logical volume
    auto muonSD_A = new MuonDetectorA("MuonSD_A");
    auto muonSD_B = new MuonDetectorB("MuonSD_B");
    auto sdManager = G4SDManager::GetSDMpointer();
    sdManager->AddNewDetector(muonSD_A);
    sdManager->AddNewDetector(muonSD_B);

    det1LV->SetSensitiveDetector(muonSD_A);
    det2LV->SetSensitiveDetector(muonSD_B);
}

void DetectorConstruction::DefineMaterials() {
    G4NistManager::Instance();
}
