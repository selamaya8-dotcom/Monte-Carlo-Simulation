#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <TH1F.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TStyle.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./make_histogram <csv_file>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string baseName = inputFile;
    size_t lastdot = baseName.find_last_of(".");
    if (lastdot != std::string::npos) baseName = baseName.substr(0, lastdot);

    std::vector<double> energyDeposits;
    std::vector<double> initialAngles;
    std::vector<double> initialEnergies;

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << inputFile << std::endl;
        return 1;
    }

    std::string line;
    int lineCount = 0; // Added declaration
    while (std::getline(file, line)) {
        if (line.empty() || line.find("EventID") != std::string::npos) continue;

        std::stringstream ss(line);
        std::string val;
        std::vector<std::string> row;

        while (std::getline(ss, val, ',')) {
            row.push_back(val);
        }

        if (row.size() >= 8) {
            try {
                energyDeposits.push_back(std::stod(row[5])); // MeV
                initialAngles.push_back(std::stod(row[6]));  // Degrees

                // Convert MeV (from Geant4) to GeV for the vector
                double energyGeV = std::stod(row[7]) / 1000.0;
                initialEnergies.push_back(energyGeV);

                lineCount++;
            } catch (...) { continue; }
        }
    }
    file.close();

    if (energyDeposits.empty()) {
        std::cerr << "Error: No data found in " << inputFile << std::endl;
        return 1;
    }

    // --- Create ROOT File early so histograms can be associated with it ---
    TFile *rootFile = new TFile((baseName + ".root").c_str(), "RECREATE");

    gStyle->SetOptStat(1111);
    TCanvas *c1 = new TCanvas("c1", "Canvas", 800, 600);

    // --- 1. Energy Deposit Histogram (0-20 MeV) ---
    TH1F *hEnergy = new TH1F("hEnergy", "Energy Deposit;Energy (MeV);Events", 100, 0, 20);
    for (double e : energyDeposits) hEnergy->Fill(e);
    hEnergy->SetLineColor(kBlue);
    hEnergy->Draw("HIST");
    c1->SaveAs((baseName + "_energy.png").c_str());
    hEnergy->Write();

    // --- 2. Initial Angle Histogram (0-90 deg) ---
    c1->Clear();
    TH1F *hAngle = new TH1F("hAngle", "Initial Muon Angle;Angle (deg);Events", 90, 0, 90);
    for (double a : initialAngles) hAngle->Fill(a);
    hAngle->SetLineColor(kRed);
    hAngle->Draw("HIST");
    c1->SaveAs((baseName + "_angle.png").c_str());
    hAngle->Write();

    // --- 3. Initial Energy Histogram (0-100 GeV) ---
    c1->Clear();
    TH1F *hGenEnergy = new TH1F("hGenEnergy", "Initial Muon Energy;Energy (GeV);Events", 40, 0, 40);
    for (double e : initialEnergies) hGenEnergy->Fill(e);
    hGenEnergy->SetLineColor(kGreen+2);
    hGenEnergy->Draw("HIST");
    c1->SaveAs((baseName + "_initial_energy.png").c_str());
    hGenEnergy->Write();

    rootFile->Close();

    std::cout << "Success! Read " << lineCount << " events." << std::endl;
    std::cout << "Generated: " << baseName << ".root and PNG files." << std::endl;

    return 0;
}
