#include <TCanvas.h>
#include <TH2D.h>
#include <TStyle.h>
#include <TColor.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

void make_image(const char* input_file = "final_accumulated.csv") {
    TCanvas *c1 = new TCanvas("c1", "Muon Scattering Reconstruction", 800, 800);
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kViridis);

    // ORIGINAL LIMITS: -55 to 55 (Total 110mm, matching your 11cm detector)
    TH2D *hSumAngles = new TH2D("hSum", "Total Scattering;X (mm);Z (mm)", 50, -55, 55, 50, -55, 55);
    TH2D *hCounts = new TH2D("hCounts", "Hit Counts;X (mm);Z (mm)", 50, -55, 55, 50, -55, 55);

    std::ifstream file(input_file);
    std::string line;

    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        if (line.empty() || line.find("EventID") != std::string::npos) continue;

        std::stringstream ss(line);
        std::string eid, x_s, z_s, angle_s, energy_s;

        std::getline(ss, eid, ','); // EventID
        std::getline(ss, x_s, ','); // xA
        std::getline(ss, z_s, ','); // zA
        std::getline(ss, angle_s, ','); // ScatteringAngle

        try {
            double x = std::stod(x_s);
            double z = std::stod(z_s);
            double angle = std::stod(angle_s);

            hSumAngles->Fill(x, z, angle);
            hCounts->Fill(x, z);
        } catch (...) { continue; }
    }

    // Divide to get the average scattering per pixel
    hSumAngles->Divide(hCounts);

    hSumAngles->SetTitle("Average Muon Scattering Map (Radians)");
    hSumAngles->Draw("COLZ");

    c1->SaveAs("scattering_reconstruction.png");
}
