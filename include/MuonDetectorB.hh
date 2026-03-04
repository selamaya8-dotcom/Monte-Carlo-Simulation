#ifndef MUONDETECTORA_HH
#define MUONDETECTORA_HH

#include "G4VSensitiveDetector.hh"
#include "MuonHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

/**
 * @brief Sensitive Detector for volume A.
 * Tracks muon energy deposition and interaction timing.
 */
class MuonDetectorA : public G4VSensitiveDetector {
public:
    MuonDetectorA(G4String name);
    virtual ~MuonDetectorA() override = default;

    // Standard Geant4 SD interface
    virtual void   Initialize(G4HCofThisEvent* hce) override;
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;

private:
    MuonHitsCollection* fHitsCollection = nullptr; // Managed by G4SDManager
    G4int fHCID = -1;                              // Hit Collection ID
};

#endif
