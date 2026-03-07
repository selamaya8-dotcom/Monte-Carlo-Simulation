#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

/**
 * @brief Handles data collection at the end of each event.
 * Consolidates hit information from multiple sensitive detectors and 
 * logs data to both the console and a CSV file.
 */
class EventAction : public G4UserEventAction
{
  public:
    EventAction();
    virtual ~EventAction() override;

    virtual void BeginOfEventAction(const G4Event* event) override;
    virtual void EndOfEventAction(const G4Event* event) override;

    // Setters for generator metadata
    void SetGenAngle(G4double val)  { fGenAngle = val; }
    void SetGenEnergy(G4double val) { fGenEnergy = val; }

  private:
    G4double fGenAngle;
    G4double fGenEnergy;
};

#endif
