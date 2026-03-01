// Compile with: g++ make_histogram.cpp `root-config --cflags --libs` -o make_histogram
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <TH1F.h>
#include <TFile.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./make_histogram <csv_file> [output_rootfile]" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = "energyDepositHistogram.root";
    if (argc > 2) outputFile = argv[2];

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << inputFile << std::endl;
        return 1;
    }

    std::vector<double> energyDeposits;
    std::string line;
    bool firstLine = true;

    double totalEdep = 0.0;
    while (std::getline(file, line)) {
      // Ensure you are skipping the header in make_histogram.cpp
      if (line.find("EventID") != std::string::npos) continue;

      std::stringstream ss(line);
      std::string eid, x_s, z_s, angle_s, energy_s;

      std::getline(ss, eid, ',');
      std::getline(ss, x_s, ',');
      std::getline(ss, z_s, ',');
      std::getline(ss, angle_s, ',');
      std::getline(ss, energy_s, ','); 

        try {
            double totalEnergy = std::stod(energy_s);
            energyDeposits.push_back(totalEnergy);
            totalEdep += totalEnergy;
        } catch (...) { continue; }
    }
    file.close();

    // Create ROOT histogram: adjust bins/range as needed
    TH1F *hist = new TH1F("hEnergyDeposit", "Total Energy Deposit per Muon Event", 400, 0, 200); // 40 bins from 0 to 20 MeV

    hist->SetXTitle("Energy Deposit (MeV)");
    hist->SetYTitle("Number of Events");

    for (double edep : energyDeposits) {
        hist->Fill(edep);
    }

    std::cout << "Total Energy Deposited: " << totalEdep << std::endl;

    // Write to ROOT file
    TFile *rootFile = new TFile(outputFile.c_str(), "RECREATE");
    hist->Write();
    rootFile->Close();

    std::cout << "Histogram written to " << outputFile << std::endl;
    return 0;
}
