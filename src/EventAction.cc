//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "EventAction.hh"

//#include "HistoManager.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4THitsMap.hh"

#include "MuonHit.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction()
:G4UserEventAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*)
{

}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event* event)
{
    auto hce = event->GetHCofThisEvent();
    if (!hce) return;

    static G4int hcID_A = -1, hcID_B = -1;
    if (hcID_A < 0) {
        hcID_A = G4SDManager::GetSDMpointer()->GetCollectionID("MuonSD_A/muonHitCollectionA");
    }
    if (hcID_B < 0) {
        hcID_B = G4SDManager::GetSDMpointer()->GetCollectionID("MuonSD_B/muonHitCollectionB");
    }

    MuonHitsCollection* hitsCollectionA = static_cast<MuonHitsCollection*>(hce->GetHC(hcID_A));
    MuonHitsCollection* hitsCollectionB = static_cast<MuonHitsCollection*>(hce->GetHC(hcID_B));

    if (!hitsCollectionA || !hitsCollectionB) return;

    // Process hitsCollectionA
    G4int nHitsA = hitsCollectionA->entries();
    G4double totalEdepA = 0.;
    for (G4int i = 0; i < nHitsA; i++) {
        auto hit = (*hitsCollectionA)[i];
        totalEdepA += hit->GetEdep();
    }

    // Process hitsCollectionB
    G4int nHitsB = hitsCollectionB->entries();
    G4double totalEdepB = 0.;
    for (G4int i = 0; i < nHitsB; i++) {
        auto hit = (*hitsCollectionB)[i];
        totalEdepB += hit->GetEdep();
    }

    const char* env_id = std::getenv("G4_RUN_ID");
    std::string fileName = (env_id) ? "hits_output_" + std::string(env_id) + ".csv" : "hits_output.csv";

    std::ofstream outfile(fileName, std::ios::app);
    if (outfile.is_open()) {
        for (G4int i = 0; i < nHitsA; i++) {
            auto hit = (*hitsCollectionA)[i];
            G4double edep = hit->GetEdep();
            G4ThreeVector pos = hit->GetPos();
            outfile << "A," << event->GetEventID() << ": "
                    << edep << ",("
                    << pos.x() << ","
                    << pos.y() << ") \n";
        }
        for (G4int i = 0; i < nHitsB; i++) {
            auto hit = (*hitsCollectionB)[i];
            G4double edep = hit->GetEdep();
            G4ThreeVector pos = hit->GetPos();
            outfile << "B," << event->GetEventID() << ": "
                    << edep << ",("
                    << pos.x() << ","
                    << pos.y() << ") \n";
        }
        outfile.close();
    } else {
        G4cerr << "Could not open hits_output.csv for writing!" << G4endl;
    }

    if (totalEdepA > 1E-9*MeV) {
        G4cout << "Total energy deposited in Detector A: " << G4BestUnit(totalEdepA, "Energy") << G4endl;
        /*
        G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
        G4int id = 0;
        analysisManager->FillNtupleIColumn(id, 0, event->GetEventID());
        analysisManager->FillNtupleDColumn(id, 1, totalEdepA);
        analysisManager->AddNtupleRow(id);
        */
    }

    if (totalEdepB > 1E-9*MeV) {
        G4cout << "Total energy deposited in Detector B: " << G4BestUnit(totalEdepB, "Energy") << G4endl;
        // Add analysis manager code for Detector B if needed
    }

    G4int nHits = nHitsA + nHitsB;
    G4double totalEdep = totalEdepA + totalEdepB;
    if (nHits>0) {
        G4cout << "EndOfEvent: " << nHits << " hits in Muon detector" << G4endl;
    }
    if (totalEdep) {
        G4cout << "Total energy deposited: " << G4BestUnit(totalEdep, "Energy") << G4endl;
    }

}
