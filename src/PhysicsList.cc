#include "PhysicsList.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4ParticleTable.hh"
#include "G4ProcessManager.hh"

#include "G4HadronElasticPhysicsHP.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4EmParameters.hh"
#include "G4DecayPhysics.hh"
#include "G4NuclideTable.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4HadronPhysicsFTFP_BERT.hh"
#include "G4HadronPhysicsQGSP_BIC_HP.hh"
#include "G4HadronInelasticQBBC.hh"
#include "G4HadronPhysicsINCLXX.hh"
#include "G4IonElasticPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4IonINCLXXPhysics.hh"

#include "G4EmStandardPhysics_option4.hh"

#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BosonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

#include "G4UserSpecialCuts.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4PhysicsListHelper.hh"



PhysicsList::PhysicsList()
:G4VModularPhysicsList()
{
  G4int verb = 1;
  SetVerboseLevel(verb);

    RegisterPhysics(new G4EmStandardPhysics_option4());

    RegisterPhysics( new G4HadronPhysicsFTFP_BERT(verb));

    RegisterPhysics(new G4DecayPhysics());

}

PhysicsList::~PhysicsList()
{ }

void PhysicsList::ContructProcess()
{
  G4VModularPhysicsList::ConstructProcess();
  RegisterPhysics(new G4StepLimiterPhysics());//for user limits
}

void PhysicsList::ConstructParticle()
{
  G4BosonConstructor  pBosonConstructor;
  pBosonConstructor.ConstructParticle();

  G4LeptonConstructor pLeptonConstructor;
  pLeptonConstructor.ConstructParticle();

  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4IonConstructor pIonConstructor;
  pIonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();
}

void PhysicsList::SetCuts()
{
  SetCutValue(1*cm, "proton");
  SetCutValue(1*cm, "e-");
  SetCutValue(1*cm, "e+");
  SetCutValue(0.1*km, "gamma");
}
