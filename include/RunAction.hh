// RunAction.hh
#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"

class HistoManager;

class RunAction : public G4UserRunAction
{
public:
  RunAction();
  virtual ~RunAction();

  virtual void BeginOfRunAction(const G4Run*) override;
  virtual void EndOfRunAction(const G4Run*) override;

private:
  HistoManager* fHistoManager = nullptr;
};

#endif
