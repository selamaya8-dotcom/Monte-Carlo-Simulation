#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

/**
 * @brief Manages the start and end of a simulation run.
 * Responsible for initializing the CSV output file and reporting run statistics.
 */
class RunAction : public G4UserRunAction
{
public:
    RunAction();
    virtual ~RunAction() override;

    virtual void BeginOfRunAction(const G4Run* run) override;
    virtual void EndOfRunAction(const G4Run* run) override;
};

#endif
