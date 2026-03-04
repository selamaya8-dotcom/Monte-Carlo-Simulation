#include "MuonDetectorB.hh"
#include "G4Step.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"

MuonDetectorB::MuonDetectorB(G4String name)
: G4VSensitiveDetector(name), fHitsCollection(nullptr), fHCID(-1) 
{
    collectionName.insert("muonHitCollectionB");
}

void MuonDetectorB::Initialize(G4HCofThisEvent* hce) 
{
    fHitsCollection = new MuonHitsCollection(SensitiveDetectorName, collectionName[0]);
    
    if (fHCID < 0) {
        fHCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
    }
    
    hce->AddHitsCollection(fHCID, fHitsCollection);
}

G4bool MuonDetectorB::ProcessHits(G4Step* step, G4TouchableHistory*) 
{
    if (!fHitsCollection) return false;
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep <= 0.) return false;

    G4StepPoint* preStepPoint = step->GetPreStepPoint();
    
    MuonHit* hit = new MuonHit();
    hit->SetEdep(edep);
    hit->SetPos(preStepPoint->GetPosition());
    hit->SetTime(preStepPoint->GetGlobalTime());

    fHitsCollection->insert(hit);
    
    return true;
}
