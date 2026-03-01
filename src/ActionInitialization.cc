#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::ActionInitialization(DetectorConstruction* detector)
 : G4VUserActionInitialization(),
   fDetector(detector)
{}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::~ActionInitialization()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


void ActionInitialization::BuildForMaster() const
{
  RunAction* runAction = new RunAction();
  SetUserAction(runAction);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void ActionInitialization::Build() const {
    // Create the primary generator for each thread
    PrimaryGeneratorAction* primary = new PrimaryGeneratorAction(fDetector);
    SetUserAction(primary);

    // Add other user actions here
    RunAction* runAction = new RunAction();
    SetUserAction(runAction);

    EventAction* event = new EventAction();
    SetUserAction(event);


}
