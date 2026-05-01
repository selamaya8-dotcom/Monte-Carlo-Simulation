#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"

ActionInitialization::ActionInitialization(DetectorConstruction* detector)
 : G4VUserActionInitialization(),
   fDetector(detector)
{}

ActionInitialization::~ActionInitialization()
{}

void ActionInitialization::BuildForMaster() const
{
  RunAction* runAction = new RunAction();
  SetUserAction(runAction);
}

void ActionInitialization::Build() const {
    PrimaryGeneratorAction* primary = new PrimaryGeneratorAction(fDetector);
    SetUserAction(primary);

    RunAction* runAction = new RunAction();
    SetUserAction(runAction);

    EventAction* event = new EventAction();
    SetUserAction(event);


}
