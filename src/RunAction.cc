// RunAction.cc
#include "RunAction.hh"
//#include "HistoManager.hh"

#include "G4Run.hh"
#include "G4AnalysisManager.hh"

RunAction::RunAction()
  : G4UserRunAction()//, fHistoManager(0)
  {

  //fHistoManager = new HistoManager();



  }

RunAction::~RunAction() {
  //delete fHistoManager;
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  G4cout << "BeginOfRunAction called!" << G4endl;

  std::ofstream outfile("hits_output.csv");
  outfile << "Detector,EventID,PosX,PosY,PosZ,MomX,MomY,MomZ,Edep,GenAngle\n";
  outfile.close();
/*
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile();
*/
}

void RunAction::EndOfRunAction(const G4Run*)
{
  /*
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();
*/
}
