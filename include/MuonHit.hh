#ifndef MuonHit_h
#define MuonHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "tls.hh"

class MuonHit : public G4VHit {
public:
    MuonHit() = default;
    MuonHit(const MuonHit&) = default;
    ~MuonHit() override = default;

    // Operators
    MuonHit& operator=(const MuonHit&) = default;
    G4bool operator==(const MuonHit&) const { return false; }

    // Memory Management: Optimized for high-frequency allocation during tracking
    inline void* operator new(size_t);
    inline void  operator delete(void*);

    // Setters
    void SetEdep(G4double de)           { fEdep = de; }
    void SetPos(G4ThreeVector pos)      { fPos = pos; }
    void SetTime(G4double t)            { fTime = t; }

    // Getters
    G4double      GetEdep() const       { return fEdep; }
    G4ThreeVector GetPos()  const       { return fPos; }
    G4double      GetTime() const       { return fTime; }

private:
    G4double      fEdep = 0.;    // Energy deposited in the step
    G4ThreeVector fPos;          // Position of the interaction (usually PreStepPoint)
    G4double      fTime = 0.;    // Global time of the interaction
};

// --- Memory Management Implementation ---

// Define the HitsCollection type for this specific hit
using MuonHitsCollection = G4THitsCollection<MuonHit>;

// Thread-local allocator to speed up the creation/deletion of hit objects
extern G4ThreadLocal G4Allocator<MuonHit>* MuonHitAllocator;

inline void* MuonHit::operator new(size_t) {
    if (!MuonHitAllocator) {
        MuonHitAllocator = new G4Allocator<MuonHit>;
    }
    return (void*)MuonHitAllocator->MallocSingle();
}

inline void MuonHit::operator delete(void* hit) {
    MuonHitAllocator->FreeSingle((MuonHit*)hit);
}

#endif
