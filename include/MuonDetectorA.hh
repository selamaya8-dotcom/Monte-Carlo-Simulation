#ifndef MUONDETECTORB_HH
#define MUONDETECTORB_HH

#include "G4VSensitiveDetector.hh"
#include "MuonHit.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

/**
 * @brief Sensitive Detector for volume B.
 */
class MuonDetectorB : public G4VSensitiveDetector {
public:
    MuonDetectorB(G4String name);
    virtual ~MuonDetectorB() override = default;

    virtual void   Initialize(G4HCofThisEvent* hce) override;
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;

private:
    MuonHitsCollection* fHitsCollection = nullptr;
    G4int fHCID = -1;
};

#endif
