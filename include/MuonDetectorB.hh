#ifndef MUONDETECTORB_HH
#define MUONDETECTORB_HH

#include "G4VSensitiveDetector.hh"
#include "MuonHit.hh"

class MuonDetectorB : public G4VSensitiveDetector {
public:
    MuonDetectorB(G4String name);
    virtual ~MuonDetectorB() = default;
    void Initialize(G4HCofThisEvent* hce) override;
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;
private:
    MuonHitsCollection* fHitsCollection = nullptr;
    G4int fHCID = -1;
};

#endif
