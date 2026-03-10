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
#include <TLegend.h>
#include <TRandom3.h> // For Noise

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./make_histogram <physics_csv> <spectrum_csv>" << std::endl;
        return 1;
    }

    // --- Noise Setup ---
    TRandom3 randGen(0);
    const double sigma_noise = 0.05; // Adjust this to match experiment spread
    const double MeV_to_Volt = 1.0 / 5.0;

    std::string physFile = argv[1];
    std::string specFile = argv[2];
    std::string baseName = physFile;
    size_t lastdot = baseName.find_last_of(".");
    if (lastdot != std::string::npos) baseName = baseName.substr(0, lastdot);

    std::vector<double> energyDeposits, initialAngles, initialEnergies;
    std::vector<double> specEnergyA, specEnergyB;
    int lineCount = 0;
    int specCount = 0;
    std::string line;

    // 1. Read Physics CSV
    std::ifstream pFile(physFile);
    if (!pFile.is_open()) {
        std::cerr << "Error: Could not open physics file: " << physFile << std::endl;
        return 1;
    }
    while (std::getline(pFile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.find("EventID") != std::string::npos) continue;
        std::stringstream ss(line);
        std::string val;
        std::vector<std::string> row;
        while (std::getline(ss, val, ',')) row.push_back(val);
        if (row.size() >= 7) {
            try {
                energyDeposits.push_back(std::stod(row[5]) * MeV_to_Volt);
                initialAngles.push_back(std::stod(row[6]));
                if(row.size() > 7) initialEnergies.push_back(std::stod(row[7]) / 1000.0);
                lineCount++;
            } catch (...) { continue; }
        }
    }
    pFile.close();

    // 2. Read Spectrum CSV with Smearing
    std::ifstream sFile(specFile);
    if (!sFile.is_open()) {
        std::cerr << "Error: Could not open spectrum file: " << specFile << std::endl;
        return 1;
    }
    while (std::getline(sFile, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.find("EventID") != std::string::npos) continue;
        std::stringstream ss(line);
        std::string val;
        std::vector<std::string> row;
        while (std::getline(ss, val, ',')) row.push_back(val);
        if (row.size() >= 3) {
            try {
                double rawA = std::stod(row[1]) * MeV_to_Volt;
                double rawB = std::stod(row[2]) * MeV_to_Volt;
                specEnergyA.push_back(randGen.Gaus(rawA, sigma_noise));
                specEnergyB.push_back(randGen.Gaus(rawB, sigma_noise));
                specCount++;
            } catch (...) { continue; }
        }
    }
    sFile.close();

    std::cout << "Read " << lineCount << " physics events and " << specCount << " spectrum entries." << std::endl;

    TCanvas *c1 = new TCanvas("c1", "Canvas", 800, 600);

    // --- A. DETECTOR COMPARISON ---
    gStyle->SetOptStat(0);
    TH1F *hDet1 = new TH1F("hDet1", "Detector Comparison;Signal [Volts];counts", 60, 0, 2);
    TH1F *hDet2 = new TH1F("hDet2", "Detector Comparison;Signal [Volts];counts", 100, 0, 2);
    for (double v : specEnergyA) hDet1->Fill(v);
    for (double v : specEnergyB) hDet2->Fill(v);

    hDet1->SetLineColor(kBlue);
    hDet1->SetFillColorAlpha(kBlue, 0.3);
    hDet1->SetFillStyle(3004);
    hDet2->SetLineColor(kRed);
    hDet2->SetFillColorAlpha(kRed, 0.3);
    hDet2->SetFillStyle(3005);

    hDet1->SetMaximum(std::max(hDet1->GetMaximum(), hDet2->GetMaximum()) * 1.2);
    hDet1->Draw("HIST E");
    hDet2->Draw("HIST E SAME");

    TLegend *leg = new TLegend(0.6, 0.7, 0.85, 0.85);
    leg->AddEntry(hDet1, "Detector 1", "f");
    leg->AddEntry(hDet2, "Detector 2", "f");
    leg->Draw();
    c1->SaveAs("recreated_spectrum.png");

    TFile *specRootFile = new TFile("detector_spectrum.root", "RECREATE");
    hDet1->Write();
    hDet2->Write();
    specRootFile->Close();

    // --- B. Total Energy Deposit ---
    c1->Clear();
    gStyle->SetOptStat(1111);
    TH1F *hEnergy = new TH1F("hEnergy", "Total Energy Deposit;Signal (Volts);Events", 100, 0, 4);
    for (double v : energyDeposits) hEnergy->Fill(v);
    hEnergy->SetLineColor(kBlue);
    hEnergy->Draw("HIST");
    c1->SaveAs((baseName + "_energy.png").c_str());

    // --- C. Initial Angle ---
    c1->Clear();
    TH1F *hAngle = new TH1F("hAngle", "Initial Muon Angle;Angle (deg);Events", 90, 0, 90);
    for (double a : initialAngles) hAngle->Fill(a);
    hAngle->SetLineColor(kRed);
    hAngle->Draw("HIST");
    c1->SaveAs((baseName + "_angle.png").c_str());

    // --- D. Initial Energy ---
    c1->Clear();
    TH1F *hGenEnergy = new TH1F("hGenEnergy", "Initial Muon Energy;Energy (GeV);Events", 40, 0, 40);
    for (double e : initialEnergies) hGenEnergy->Fill(e);
    hGenEnergy->SetLineColor(kGreen+2);
    hGenEnergy->Draw("HIST");
    c1->SaveAs((baseName + "_initial_energy.png").c_str());

    delete hDet1; delete hDet2; delete hEnergy; delete hAngle; delete hGenEnergy; delete c1;
    return 0;
}
