#include "RunAction.hh"
#include "G4Run.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include <fstream>

RunAction::RunAction() : G4UserRunAction()
{}

RunAction::~RunAction() 
{}

void RunAction::BeginOfRunAction(const G4Run* run)
{
    G4cout << "### Run " << run->GetRunID() << " start." << G4endl;

    // Initialize CSV and write header
    std::ofstream outfile("hits_output.csv");
    if (outfile.is_open()) {
        outfile << "Detector,EventID,PosX,PosY,PosZ,MomX,MomY,MomZ,Edep,GenAngle\n";
        outfile.close(); 
    } else {
        G4cerr << "Error: Could not create output CSV file!" << G4endl;
    }
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4int nofEvents = run->GetNumberOfEvent();
    if (nofEvents == 0) return;

    // Print Run Summary
    G4cout << G4endl << "------- End of Run -------" << G4endl;
    G4cout << " Run ID:         " << run->GetRunID() << G4endl;
    G4cout << " Total Events:   " << nofEvents << G4endl;
    G4cout << " Output saved to: hits_output.csv" << G4endl;
    G4cout << "---------------------------" << G4endl << G4endl;
}
