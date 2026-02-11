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
    /*
    G4double protons = 6, mole = 12.0*g/mole, density = 16.0*g/cm3; //mole and protons from carbon
    G4Material* concrete = new G4Material("Carbon", protons, mole, density);
    //auto concrete = nist->FindOrBuildMaterial("G4_Np");
    auto air      = nist->FindOrBuildMaterial("G4_AIR");
    */
    // Define basic elements
    G4Element* elSi = new G4Element("Silicon", "Si", 14., 28.0855*g/mole);
    G4Element* elMg = new G4Element("Magnesium", "Mg", 12., 24.305*g/mole);
    G4Element* elAl = new G4Element("Aluminum", "Al", 13., 26.9815*g/mole);
    G4Element* elCa = new G4Element("Calcium", "Ca", 20., 40.078*g/mole);
    G4Element* elFe = new G4Element("Iron", "Fe", 26., 55.845*g/mole);
    G4Element* elNa = new G4Element("Sodium", "Na", 11., 22.9897*g/mole);
    G4Element* elTi = new G4Element("Titanium", "Ti", 22., 47.867*g/mole);
    G4Element* elO  = new G4Element("Oxygen", "O", 8., 16.00*g/mole);

    // Define compound oxides
    G4Material* SiO2 = new G4Material("SiliconDioxide", 2.65*g/cm3, 2);
    SiO2->AddElement(elSi, 1);
    SiO2->AddElement(elO, 2);

    G4Material* MgO = new G4Material("MagnesiumOxide", 3.58*g/cm3, 2);
    MgO->AddElement(elMg, 1);
    MgO->AddElement(elO, 1);

    G4Material* Al2O3 = new G4Material("AluminumOxide", 3.95*g/cm3, 2);
    Al2O3->AddElement(elAl, 2);
    Al2O3->AddElement(elO, 3);

    G4Material* CaO = new G4Material("CalciumOxide", 3.34*g/cm3, 2);
    CaO->AddElement(elCa, 1);
    CaO->AddElement(elO, 1);

    G4Material* FeO = new G4Material("IronOxide", 5.70*g/cm3, 2);
    FeO->AddElement(elFe, 1);
    FeO->AddElement(elO, 1);

    G4Material* Na2O = new G4Material("SodiumOxide", 2.27*g/cm3, 2);
    Na2O->AddElement(elNa, 2);
    Na2O->AddElement(elO, 1);

    G4Material* TiO2 = new G4Material("TitaniumDioxide", 4.23*g/cm3, 2);
    TiO2->AddElement(elTi, 1);
    TiO2->AddElement(elO, 2);

    // Define the soil mixture
    // G4Material* Soil = new G4Material("Soil", 2.0*g/cm3, 7);//fSoilDensity

    G4cout << "HERE!!!!! " << fSoilDensity/(g/cm3) << " g/cm3" << G4endl;

    G4Material* Soil = new G4Material("Soil", fSoilDensity, 7);//fSoilDensity
    Soil->AddMaterial(SiO2, 45.0*perCent);
    Soil->AddMaterial(MgO, 35.0*perCent);
    Soil->AddMaterial(Al2O3, 7.0*perCent);
    Soil->AddMaterial(CaO, 7.0*perCent);
    Soil->AddMaterial(FeO, 5.0*perCent);
    Soil->AddMaterial(Na2O, 0.7*perCent);
    Soil->AddMaterial(TiO2, 0.3*perCent);

    auto air      = nist->FindOrBuildMaterial("G4_AIR");


    // World
    auto worldS = new G4Box("World", 50*m, 50*m, 50*m);
    auto worldLV = new G4LogicalVolume(worldS, air, "World");
    auto worldPV = new G4PVPlacement(nullptr, {}, worldLV, "World", nullptr, false, 0);
    fWorldHx = worldS->GetXHalfLength();

    // Ground
    auto groundS = new G4Box("Ground", 50*m, 50*m, 50*m);
    auto groundLV = new G4LogicalVolume(groundS, Soil, "Ground");
    new G4PVPlacement(nullptr, G4ThreeVector(0, -50*m, 0), groundLV, "Ground", worldLV, false, 0);

    // Building
    /*
    const G4double t = 3.*m; // wall thickness
    const G4double eps = 1.*mm; // avoid coincident surfaces
    const G4double hx = 30.*m;
    const G4double hy = 20.*m;
    const G4double hz = 20.*m;
    const G4ThreeVector Bpos(40.*m, 20.*m, -60.*m); // Building

    auto outer = new G4Box("B_outer", hx, hy, hz);
    auto inner = new G4Box("B_inner", hx - t - eps, hy - t - eps, hz - t - eps);
    auto shellSolid = new G4SubtractionSolid("BuildingShell", outer, inner);
    auto shellLV = new G4LogicalVolume(shellSolid, Soil, "Building");
    auto outerLV    = new G4LogicalVolume(outer, air, "OuterLV");
    auto shellPV  = new G4PVPlacement(nullptr, Bpos, shellLV, "Building", worldLV, false, 0);
    fBuildHx = outer->GetXHalfLength();
    fBuildHy = outer->GetYHalfLength();
    fBuildHz = outer->GetZHalfLength();
    fBuildCenter = Bpos;
    */


/*
    // Concrete block 1m x 1m x 1m
    G4double blockSize = 1.0*m;
    auto concreteBlockSolid = new G4Box("ConcreteBlock", 0.5*blockSize, 0.5*blockSize, 0.5*blockSize);
    auto concreteMaterial = Soil;
    auto concreteBlockLogical = new G4LogicalVolume(concreteBlockSolid, concreteMaterial, "ConcreteBlockLV");
    G4ThreeVector concreteBlockPosition = G4ThreeVector(0, 0.5*m, -0.5*m);
    new G4PVPlacement(nullptr, concreteBlockPosition, concreteBlockLogical, "ConcreteBlock", worldLV, false, 0);
*/


    // Concrete Sphere (replacing the block)
    G4double sphereRadius = 0.5 * m;
    auto concreteSphereSolid = new G4Sphere(
        "ConcreteSphere",           // name
        0.*cm,                      // inner radius
        sphereRadius,               // outer radius
        0.*deg, 360.*deg,           // phi start and span
        0.*deg, 180.*deg            // theta start and span
    );

    auto concreteMaterial = Soil;
    auto concreteSphereLogical = new G4LogicalVolume(concreteSphereSolid, concreteMaterial, "ConcreteSphereLV");

    // Position it at the same spot the block was
    G4ThreeVector spherePosition = G4ThreeVector(0, 0.5*m, -0.5*m);
    new G4PVPlacement(nullptr, spherePosition, concreteSphereLogical, "ConcreteSphere", worldLV, false, 0);


    /*
    auto detS = new G4Box("Det", 20*m, 20*m, 20*m);
    fDetectorLogic = new G4LogicalVolume(detS, nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"), "Det");
    G4ThreeVector detPlacment = G4ThreeVector(0*m, 20*m, 50*m);
    new G4PVPlacement(nullptr, detPlacment, fDetectorLogic, "Det", worldLV, false, 0);
    fDetHx = detS->GetXHalfLength();
    fDetHy = detS->GetYHalfLength();
    fDetHz = detS->GetZHalfLength();
    fDetCenter = detPlacment;
    */

    // Detector


    G4double boxXY = 11*cm, boxZ = 52*cm;
    auto outerBox = new G4Box("OuterBox", boxXY/2, boxXY/2, boxZ/2);
    auto innerBox = new G4Box("InnerBox", boxXY/2 - 1*cm, boxXY/2 - 1*cm, boxZ/2  - 1*cm);
    auto detS = new G4SubtractionSolid("Shell", outerBox, innerBox, nullptr, G4ThreeVector(0,0,0));
    G4Material* cardboard = nist->FindOrBuildMaterial("G4_WOOD");
    if (!cardboard) {
      G4Element* C = nist->FindOrBuildElement("C");
      G4Element* H = nist->FindOrBuildElement("H");
      G4Element* O = nist->FindOrBuildElement("O");
      cardboard = new G4Material("CardBoard", 0.7*g/cm3, 3);
      cardboard->AddElement(C, 6);
      cardboard->AddElement(H, 10);
      cardboard->AddElement(O, 5);
    }
    auto detLV = new G4LogicalVolume(detS, cardboard, "Box");
    G4ThreeVector detPlacment = G4ThreeVector(0, boxXY/2, boxZ/2); //boxXY/2, boxZ/2
    new G4PVPlacement(nullptr, detPlacment, detLV, "Box", worldLV, false, 0);
    fDetHx = boxXY/2;
    fDetHy = boxXY/2;
    fDetHz = boxZ/2;
    fDetCenter = detPlacment;

    G4double detXY = 10*cm, detZ = 2*cm;//10,2
    auto det1S = new G4Box("Det1", detXY/2, detXY/2, detZ/2);
    auto det2S = new G4Box("Det2", detXY/2, detXY/2, detZ/2);
    det1LV = new G4LogicalVolume(det1S, nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"), "Det1");
    det2LV = new G4LogicalVolume(det2S, nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"), "Det2");

    G4double wall = 1*cm;
    G4ThreeVector det1Placment = G4ThreeVector(wall, wall, boxZ/2-wall-detZ/2);
    G4ThreeVector det2Placment = G4ThreeVector(wall, wall, -boxZ/2+wall+detZ/2);
    new G4PVPlacement(nullptr, det1Placment, det1LV, "Det1", detLV, false, 0);
    new G4PVPlacement(nullptr, det2Placment, det2LV, "Det2", detLV, false, 0);

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
