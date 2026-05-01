#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"


class EventAction : public G4UserEventAction
{
  public:
    EventAction();
    virtual ~EventAction() override;

    virtual void BeginOfEventAction(const G4Event* event) override;
    virtual void EndOfEventAction(const G4Event* event) override;

    void SetGenAngle(G4double val)  { fGenAngle = val; }
    void SetGenEnergy(G4double val) { fGenEnergy = val; }
    void SetGenPosition(G4ThreeVector pos) { fGenPosition = pos; }
    G4ThreeVector GetGenPosition() const { return fGenPosition; }

  private:
    G4double fGenAngle;
    G4double fGenEnergy;
    G4ThreeVector fGenPosition;
};

#endif
