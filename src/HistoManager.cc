#include "HistoManager.hh"
#include "G4UnitsTable.hh"
#include "globals.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HistoManager::HistoManager()
  : fFileName("DroneMuonScan")
{
  Book();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HistoManager::~HistoManager()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void HistoManager::Book()
{
  G4AnalysisManager* analysis = G4AnalysisManager::Instance();

  analysis->SetDefaultFileType("root");
  analysis->SetFileName(fFileName);
  analysis->SetVerboseLevel(1);
  analysis->SetActivation(true);
  analysis->SetNtupleActivation(true);

  analysis->CreateNtuple("MuonHits","MuonHits");
  analysis->CreateNtupleIColumn("EventID");
  analysis->CreateNtupleDColumn("edep");
  analysis->FinishNtuple(0);

  G4cout << "HistoManager::Book(): histograms are booked" << G4endl;


}
