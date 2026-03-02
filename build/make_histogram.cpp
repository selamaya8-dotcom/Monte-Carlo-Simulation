// Compile with: g++ make_histogram.cpp `root-config --cflags --libs` -o make_histogram
//make less bin
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
        if (line.empty() || line.find("EventID") != std::string::npos) continue;

        std::stringstream ss(line);
        std::string eid, x_s, y_s, z_s, angle_s, energy_s;

        // Correctly parse the 6 columns of combined_hits.csv
        std::getline(ss, eid, ',');     // 1: EventID
        std::getline(ss, x_s, ',');     // 2: PosX
        std::getline(ss, y_s, ',');     // 3: PosY
        std::getline(ss, z_s, ',');     // 4: PosZ
        std::getline(ss, angle_s, ','); // 5: GenAngle
        std::getline(ss, energy_s, ','); // 6: TotalEnergy

        try {
            if (!energy_s.empty()) {
                double totalEnergy = std::stod(energy_s);
                energyDeposits.push_back(totalEnergy);
                totalEdep += totalEnergy;
            }
        } catch (...) { continue; }
    }
    file.close();

    // Create ROOT histogram: adjust bins/range as needed
    // 40 bins of 5 MeV each
    TH1F *hist = new TH1F("hEnergyDeposit", "Total Energy Deposit per Muon Event", 40, 0, 100);
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
