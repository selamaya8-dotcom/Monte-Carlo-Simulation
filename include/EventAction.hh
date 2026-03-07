#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class EventAction : public G4UserEventAction
{
  public:
    EventAction();
   ~EventAction();

   void SetGenAngle(G4double val) { fGenAngle = val; }
   void SetGenEnergy(G4double val) { fGenEnergy = val; }

    virtual void BeginOfEventAction(const G4Event*);
    virtual void   EndOfEventAction(const G4Event*);
  private:
    G4double fGenAngle;
    G4double fGenEnergy;


};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
