#ifndef MuonHit_h
#define MuonHit_h 1

#include "G4VHit.hh"
#include "G4Allocator.hh"
#include "G4THitsCollection.hh"
#include "G4ThreeVector.hh"

class MuonHit : public G4VHit {
public:
    MuonHit() = default;
    MuonHit(const MuonHit& right) = default;
    ~MuonHit() override = default;
    MuonHit& operator=(const MuonHit& right) = default;
    G4bool operator==(const MuonHit& right) const { return false; }

    inline void* operator new(size_t);
    inline void operator delete(void*);

    void SetEdep(G4double e) { fEdep = e; }
    G4double GetEdep() const { return fEdep; }

    void SetPos(const G4ThreeVector& pos) { fPos = pos; }
    G4ThreeVector GetPos() const { return fPos; }

    void SetTime(G4double t) { fTime = t; }
    G4double GetTime() const { return fTime; }

    void SetMomentum(const G4ThreeVector& mom) { fMomentum = mom; }
    G4ThreeVector GetMomentum() const { return fMomentum; }

private:
    G4double fEdep = 0.;
    G4ThreeVector fPos;
    G4double fTime = 0.;
    G4ThreeVector fMomentum;
};

using MuonHitsCollection = G4THitsCollection<MuonHit>;
extern G4ThreadLocal G4Allocator<MuonHit>* MuonHitAllocator;

inline void* MuonHit::operator new(size_t) {
    if (!MuonHitAllocator) MuonHitAllocator = new G4Allocator<MuonHit>;
    return (void*)MuonHitAllocator->MallocSingle();
}
inline void MuonHit::operator delete(void* hit) {
    MuonHitAllocator->FreeSingle((MuonHit*)hit);
}

#endif
