#include "RunAction.hh"

#include "G4Run.hh"
#include "G4AnalysisManager.hh"

RunAction::RunAction()
  : G4UserRunAction()
  {
  }

RunAction::~RunAction() {
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  G4cout << "BeginOfRunAction called!" << G4endl;

  std::ofstream outfile("hits_output.csv");
  outfile << "Detector,EventID,PosX,PosY,PosZ,MomX,MomY,MomZ,Edep,GenAngle,GenEnergy,GenPosX,GenPosY,GenPosZ\n";
  outfile.close();

}

void RunAction::EndOfRunAction(const G4Run*)
{

}
