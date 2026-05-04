
#include "G4Types.hh"
//#include "G4Random.hh" 
#include <ctime>

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4SteppingVerbose.hh"
#include "Randomize.hh"

#include "CLHEP/Random/RanecuEngine.h"

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"


int main(int argc,char** argv) {

  G4UIExecutive* ui = 0;
  if (argc == 1) ui = new G4UIExecutive(argc,argv);

  //choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);

  long seeds[2];
  time_t systime = time(NULL);

  seeds[0] = (long) systime;

  seeds[1] = (long) (systime * G4UniformRand());

  G4Random::setTheSeeds(seeds);

  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  auto runManager = G4RunManagerFactory::CreateRunManager();
  runManager->SetNumberOfThreads(1);

  DetectorConstruction* det= new DetectorConstruction;
  runManager->SetUserInitialization(det);

  PhysicsList* phys = new PhysicsList;
  runManager->SetUserInitialization(phys);

  runManager->SetUserInitialization(new ActionInitialization(det));

  G4VisManager* visManager = nullptr;

  G4UImanager* UImanager = G4UImanager::GetUIpointer();


  if (ui)  {
   //interactive mode
   visManager = new G4VisExecutive;
   visManager->Initialize();
   UImanager->ApplyCommand("/control/execute debug.mac");
   ui->SessionStart();
   delete ui;
  }
  else  {
   //batch mode
   G4String command = "/control/execute ";
   G4String fileName = argv[1];
   UImanager->ApplyCommand(command+fileName);
  }

  delete visManager;
  delete runManager;
}
