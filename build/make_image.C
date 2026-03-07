#include <TCanvas.h>
#include <TH2D.h>
#include <TStyle.h>
#include <TColor.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

void make_image(const char* input_file = "combined_hits.csv") {
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kTemperatureMap);

    TH2D *hSumAngles = new TH2D("hSum", "Total Scattering;X (mm);Z (mm)", 40, -45, 45, 40, -45, 45);
    TH2D *hCounts = new TH2D("hCounts", "Hit Counts;X (mm);Z (mm)", 40, -45, 45, 40, -45, 45);

    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << input_file << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string val;
        std::vector<double> row;

        while (std::getline(ss, val, ',')) {
            try { row.push_back(std::stod(val)); } catch (...) { continue; }
        }

        if (row.size() >= 5) {
            double x = row[1];
            double z = row[3];
            double angle = row[4];

            if (x == 0.0 && z == 0.0) continue;

            // Apply Fiducial Cut to remove edge reflections
            if (TMath::Abs(x) > 50.0 || TMath::Abs(z) > 50.0) continue;

            hSumAngles->Fill(x, z, angle);
            hCounts->Fill(x, z);
        }
    }
    file.close();

    // --- IMAGE 1: Hit Density (Linear) ---
    TCanvas *c1 = new TCanvas("c1", "Hit Density Linear", 800, 800);
    c1->SetRightMargin(0.15);
    hCounts->Draw("COLZ");
    c1->SaveAs("hit_density_linear.png");

    // --- IMAGE 2: Hit Density (Logarithmic) ---
    TCanvas *c2 = new TCanvas("c2", "Hit Density Log", 800, 800);
    c2->SetRightMargin(0.15);
    c2->SetLogz();
    hCounts->SetMinimum(1); // Required for Log scale
    hCounts->Draw("COLZ");
    c2->SaveAs("hit_density_log.png");

    // Prepare Average Angle Histogram
    TH2D *hAvgAngle = (TH2D*)hSumAngles->Clone("hAvgAngle");
    hAvgAngle->Divide(hCounts);

    // --- IMAGE 3: Average Angle (Linear) ---
    TCanvas *c3 = new TCanvas("c3", "Angular Linear", 800, 800);
    c3->SetRightMargin(0.15);
    hAvgAngle->SetMinimum(0.001);
    hAvgAngle->SetMaximum(0.06);
    hAvgAngle->Draw("COLZ");
    c3->SaveAs("reconstruction_linear.png");

    // --- IMAGE 4: Average Angle (Logarithmic) ---
    TCanvas *c4 = new TCanvas("c4", "Angular Log", 800, 800);
    c4->SetRightMargin(0.15);
    c4->SetLogz();
    hAvgAngle->Draw("COLZ");
    c4->SaveAs("reconstruction_log.png");

    std::cout << "Success! 4 images created." << std::endl;
}
