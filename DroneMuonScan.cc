#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4SteppingVerbose.hh"
#include "Randomize.hh"
#include "CLHEP/Random/RanecuEngine.h"

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

#include <ctime>

/**
 * @brief Main function. 
 * Usage: 
 * ./simulation            (Interactive mode with GUI)
 * ./simulation macro.mac  (Batch mode)
 */
int main(int argc, char** argv) {

    // 1. Setup UI Session (Interactive mode if no macro argument is provided)
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    // 2. Random Engine Configuration
    // Use RanecuEngine and seed it using the current system time
    G4Random::setTheEngine(new CLHEP::RanecuEngine);
    
    long seeds[2];
    time_t systime = time(nullptr);
    seeds[0] = (long)systime;
    seeds[1] = (long)(systime * G4UniformRand()); 
    G4Random::setTheSeeds(seeds);

    // 3. Verbosity Settings
    G4SteppingVerbose::UseBestUnit(4);

    // 4. Construct the Run Manager (Auto-selects MT or Sequential based on build)
    auto* runManager = G4RunManagerFactory::CreateRunManager();
    runManager->SetNumberOfThreads(1); // Set to 1 for consistency with non-MT ActionInitialization

    // 5. Mandatory Initialization Classes
    auto* detConstruction = new DetectorConstruction();
    runManager->SetUserInitialization(detConstruction);

    auto* physicsList = new PhysicsList();
    runManager->SetUserInitialization(physicsList);

    auto* actionInit = new ActionInitialization(detConstruction);
    runManager->SetUserInitialization(actionInit);

    // 6. Visualization and UI Management
    auto* visManager = new G4VisExecutive;
    visManager->Initialize();

    auto* UImanager = G4UImanager::GetUIpointer();

    if (ui) {
        // Interactive Mode: Execute startup macro and start UI session
        UImanager->ApplyCommand("/control/execute debug.mac");
        ui->SessionStart();
        delete ui;
    } else {
        // Batch Mode: Execute the macro file provided in the command line argument
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        UImanager->ApplyCommand(command + fileName);
    }

    // 7. Job Termination
    delete visManager;
    delete runManager;

    return 0;
}
