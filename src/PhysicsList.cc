#include "PhysicsList.hh"

// General Geant4 includes
#include "G4SystemOfUnits.hh"
#include "G4ParticleTable.hh"

// Registered Physics Constructors
#include "G4EmStandardPhysics_option4.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"
#include "G4DecayPhysics.hh"
#include "G4StepLimiterPhysics.hh"

// Particle Constructors
#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
  G4int verb = 1;
  SetVerboseLevel(verb);

  // EM Physics: Option 4 is optimized for high-precision EM and multiple scattering
  RegisterPhysics(new G4EmStandardPhysics_option4(verb));

  // Hadron Inelastic: FTFP_BERT is the standard for high-energy hadronic interactions
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT(verb));

  // Decay: Enables decay of unstable particles (muons, pions, etc.)
  RegisterPhysics(new G4DecayPhysics(verb));
  
  // Step Limiter: Ensures user-defined step limits in volumes are respected
  RegisterPhysics(new G4StepLimiterPhysics(verb));
}

PhysicsList::~PhysicsList() 
{ }

void PhysicsList::ConstructParticle()
{
  // Construct all standard particle types to support registered physics processes
  G4BosonConstructor::ConstructParticle();
  G4LeptonConstructor::ConstructParticle();
  G4MesonConstructor::ConstructParticle();
  G4BaryonConstructor::ConstructParticle();
  G4IonConstructor::ConstructParticle();
  G4ShortLivedConstructor::ConstructParticle();
}

void PhysicsList::ConstructProcess()
{
  // Invokes the ConstructProcess of all registered modular physics
  G4VModularPhysicsList::ConstructProcess();
  RegisterPhysics(new G4StepLimiterPhysics());
}

void PhysicsList::SetCuts()
{
  /** * Production cuts define the distance a secondary particle must be able 
   * to travel before it is tracked. 
   */
  
  // Set a standard 1mm cut for EM secondaries and gammas
  SetCutValue(1.0*mm, "gamma");
  SetCutValue(1.0*mm, "e-");
  SetCutValue(1.0*mm, "e+");

  // Protons usually have a small or zero cut to ensure low-energy recoil tracking
  SetCutValue(0.1*mm, "proton");

  // Log the cut values to the console
  if (verboseLevel > 0) DumpCutValuesTable();
}
