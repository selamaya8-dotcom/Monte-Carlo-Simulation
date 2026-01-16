#ifndef MUONDETECTORA_HH
#define MUONDETECTORA_HH

#include "G4VSensitiveDetector.hh"
#include "MuonHit.hh"

class MuonDetectorA : public G4VSensitiveDetector {
public:
    MuonDetectorA(G4String name);
    virtual ~MuonDetectorA() = default;
    void Initialize(G4HCofThisEvent* hce) override;
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;

private:
    MuonHitsCollection* fHitsCollection = nullptr;
    G4int fHCID = -1;

};

#endif
