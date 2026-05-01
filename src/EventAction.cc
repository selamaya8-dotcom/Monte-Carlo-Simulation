#include "EventAction.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "MuonHit.hh"
#include <fstream>

EventAction::EventAction() : G4UserEventAction(), fGenAngle(0.) {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*) {}

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

    G4int nHitsA = hitsCollectionA->entries();
    G4double totalEdepA = 0.;
    for (G4int i = 0; i < nHitsA; i++) {
        auto hit = (*hitsCollectionA)[i];
        totalEdepA += hit->GetEdep();
    }

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
            outfile << "A," << event->GetEventID() << ","
                    << hit->GetPos().x() << "," << hit->GetPos().y() << "," << hit->GetPos().z() << ","
                    << hit->GetMomentum().x() << "," << hit->GetMomentum().y() << "," << hit->GetMomentum().z() << ","
                    << hit->GetEdep() << ","
                    << fGenAngle << ","
                    << fGenEnergy << ","
                    << fGenPosition.x() << "," << fGenPosition.y() << "," << fGenPosition.z() << "\n";
        }

        for (G4int i = 0; i < nHitsB; i++) {
            auto hit = (*hitsCollectionB)[i];
            outfile << "B," << event->GetEventID() << ","
                    << hit->GetPos().x() << "," << hit->GetPos().y() << "," << hit->GetPos().z() << ","
                    << hit->GetMomentum().x() << "," << hit->GetMomentum().y() << "," << hit->GetMomentum().z() << ","
                    << hit->GetEdep() << ","
                    << fGenAngle << ","
                    << fGenEnergy << ","
                    << fGenPosition.x() << "," << fGenPosition.y() << "," << fGenPosition.z() << "\n";

        }
        outfile.close();
    }
    else {
        G4cerr << "Could not open " << fileName << " for writing!" << G4endl;
    }

    if (totalEdepA > 1E-9*MeV) {
        G4cout << "Total energy deposited in Detector A: " << G4BestUnit(totalEdepA, "Energy") << G4endl;
    }

    if (totalEdepB > 1E-9*MeV) {
        G4cout << "Total energy deposited in Detector B: " << G4BestUnit(totalEdepB, "Energy") << G4endl;
    }

    G4int nHitsTotal = nHitsA + nHitsB;
    G4double totalEdep = totalEdepA + totalEdepB;

    if (nHitsTotal > 0) {
        G4cout << "EndOfEvent: " << nHitsTotal << " hits in Muon detector" << G4endl;
    }
    if (totalEdep > 0) {
        G4cout << "Total energy deposited: " << G4BestUnit(totalEdep, "Energy") << G4endl;
    }
} 
