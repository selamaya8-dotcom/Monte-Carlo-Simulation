#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

/**
 * @brief Registers user action classes to the Geant4 kernel.
 * BuildForMaster() is used for global run accounting (Multi-threading).
 * Build() is used to define actions for individual worker threads.
 */
class ActionInitialization : public G4VUserActionInitialization 
{
  public:
    ActionInitialization(DetectorConstruction* detector);
    virtual ~ActionInitialization() override;

    virtual void BuildForMaster() const override;
    virtual void Build() const override;

  private:
    DetectorConstruction* fDetector;
};

#endif
