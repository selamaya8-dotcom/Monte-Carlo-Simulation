#include "EventAction.hh"
#include "MuonHit.hh"

#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>
#include <cstdlib>

EventAction::EventAction() 
: G4UserEventAction(), fGenAngle(0.), fGenEnergy(0.) 
{}

EventAction::~EventAction() 
{}

void EventAction::BeginOfEventAction(const G4Event*) 
{}

void EventAction::EndOfEventAction(const G4Event* event)
{
    // 1. Retrieve Hit Collections
    auto hce = event->GetHCofThisEvent();
    if (!hce) return;

    static G4int hcID_A = -1;
    static G4int hcID_B = -1;
    
    if (hcID_A < 0) hcID_A = G4SDManager::GetSDMpointer()->GetCollectionID("MuonSD_A/muonHitCollectionA");
    if (hcID_B < 0) hcID_B = G4SDManager::GetSDMpointer()->GetCollectionID("MuonSD_B/muonHitCollectionB");

    auto hitsCollectionA = static_cast<MuonHitsCollection*>(hce->GetHC(hcID_A));
    auto hitsCollectionB = static_cast<MuonHitsCollection*>(hce->GetHC(hcID_B));

    if (!hitsCollectionA || !hitsCollectionB) return;

    // 2. Prepare File Output
    const char* env_id = std::getenv("G4_RUN_ID");
    std::string fileName = (env_id) ? "hits_output_" + std::string(env_id) + ".csv" : "hits_output.csv";

    // Open in append mode
    std::ofstream outfile(fileName, std::ios::app);
    if (!outfile.is_open()) {
        G4Exception("EventAction::EndOfEventAction", "FileError", JustWarning, "Could not open CSV output file!");
        return; 
    }

    // Write header only if this is the first event of the run
    if (event->GetEventID() == 0) {
        outfile << "Detector,EventID,PosX,PosY,PosZ,Edep,GenAngle,GenEnergy\n";
    }

    G4double totalEdepA = 0.0;
    G4double totalEdepB = 0.0;

    // 3. Process Detector A (Combined energy sum and file write)
    G4int nHitsA = hitsCollectionA->entries();
    for (G4int i = 0; i < nHitsA; i++) {
        auto hit = (*hitsCollectionA)[i];
        totalEdepA += hit->GetEdep();

        outfile << "A," << event->GetEventID() << ","
                << hit->GetPos().x() << "," << hit->GetPos().y() << "," << hit->GetPos().z() << ","
                << hit->GetEdep() << ","
                << fGenAngle << ","
                << fGenEnergy << "\n"; 
    }

    // 4. Process Detector B (Combined energy sum and file write)
    G4int nHitsB = hitsCollectionB->entries();
    for (G4int i = 0; i < nHitsB; i++) {
        auto hit = (*hitsCollectionB)[i];
        totalEdepB += hit->GetEdep();

        outfile << "B," << event->GetEventID() << ","
                << hit->GetPos().x() << "," << hit->GetPos().y() << "," << hit->GetPos().z() << ","
                << hit->GetEdep() << ","
                << fGenAngle << ","
                << fGenEnergy << "\n";
    }
    
    outfile.close(); 

    // 5. Console Output Logic
    G4int nHitsTotal = nHitsA + nHitsB;
    G4double totalEdep = totalEdepA + totalEdepB;

    if (nHitsTotal > 0) {
        G4cout << "--- End of Event " << event->GetEventID() << " ---" << G4endl;
        
        if (totalEdepA > 0) {
            G4cout << " Detector A: " << G4BestUnit(totalEdepA, "Energy") << G4endl;
        }
        if (totalEdepB > 0) {
            G4cout << " Detector B: " << G4BestUnit(totalEdepB, "Energy") << G4endl;
        }
        
        G4cout << " Total: " << nHitsTotal << " hits, " 
               << G4BestUnit(totalEdep, "Energy") << " deposited." << G4endl;
    }
}
