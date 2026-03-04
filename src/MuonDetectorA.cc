#include "MuonDetectorA.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"

// Removed G4TouchableHistory.hh include as it is no longer used in the body

MuonDetectorA::MuonDetectorA(G4String name)
: G4VSensitiveDetector(name), fHitsCollection(nullptr), fHCID(-1) 
{
    collectionName.insert("muonHitCollectionA");
}

void MuonDetectorA::Initialize(G4HCofThisEvent* hce) 
{
    // Create new hits collection at the start of each event
    fHitsCollection = new MuonHitsCollection(SensitiveDetectorName, collectionName[0]);
    
    // Get unique ID for this collection from the SD Manager if not already set
    if (fHCID < 0) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
    
    // Add collection to the event
    hce->AddHitsCollection(fHCID, fHitsCollection);
}

G4bool MuonDetectorA::ProcessHits(G4Step* step, G4TouchableHistory*) 
{
    if (!fHitsCollection) return false;
    // Energy deposit check to filter out non-interacting particles
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.) return false;

    // Extract information from the PreStepPoint (start of the step)
    G4StepPoint* preStepPoint = step->GetPreStepPoint();
    
    // Create and populate a new Hit object
    MuonHit* hit = new MuonHit();
    hit->SetEdep(edep);
    hit->SetPos(preStepPoint->GetPosition());
    hit->SetTime(preStepPoint->GetGlobalTime());

    // Insert hit into the event-specific collection
    fHitsCollection->insert(hit);
    
    return true;
}
