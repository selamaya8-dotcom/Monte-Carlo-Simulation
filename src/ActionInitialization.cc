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
  // Master thread only handles RunAction (for merging global results)
  RunAction* runAction = new RunAction();
  SetUserAction(runAction);
}

void ActionInitialization::Build() const {
    // Primary Generator: Sources particles for each thread
    PrimaryGeneratorAction* primary = new PrimaryGeneratorAction(fDetector);
    SetUserAction(primary);

    // RunAction: Handles start/end of simulation runs
    RunAction* runAction = new RunAction();
    SetUserAction(runAction);

    // EventAction: Handles data collection at the end of each event
    EventAction* event = new EventAction();
    SetUserAction(event);


}
