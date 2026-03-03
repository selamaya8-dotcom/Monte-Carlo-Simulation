#include <TCanvas.h>
#include <TH2D.h>
#include <TStyle.h>
#include <TColor.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

void make_image(const char* input_file = "combined_hits.csv") {
    TCanvas *c1 = new TCanvas("c1", "Muon Scattering Reconstruction", 800, 800);
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kTemperatureMap);

    // Bins and Limits matching your detector size
    TH2D *hSumAngles = new TH2D("hSum", "Total Scattering;X (mm);Z (mm)", 50, -55, 55, 50, -55, 55);
    TH2D *hCounts = new TH2D("hCounts", "Hit Counts;X (mm);Z (mm)", 50, -55, 55, 50, -55, 55);

    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << input_file << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line); // Skip header

    while (std::getline(file, line)) {
        if (line.empty() || line.find("EventID") != std::string::npos) continue;

        std::stringstream ss(line);
        std::string eid, x_s, y_s, z_s, angle_s, energy_s;

        // COLUMN MAPPING FOR 6-COLUMN FORMAT:
        // 1: EventID, 2: PosX, 3: PosY, 4: PosZ, 5: GenAngle, 6: TotalEnergy
        std::getline(ss, eid, ',');      // Column 1
        std::getline(ss, x_s, ',');      // Column 2
        std::getline(ss, y_s, ',');      // Column 3 (This was missing in your snippet!)
        std::getline(ss, z_s, ',');      // Column 4
        std::getline(ss, angle_s, ',');  // Column 5
        std::getline(ss, energy_s, ','); // Column 6

        try {
            double x = std::stod(x_s);
            double z = std::stod(z_s);
            double angle = std::stod(angle_s);

            // FILTER: Ignore events that are exactly at (0,0)
            // This removes the "fake" central cluster
            if (x == 0.0 && z == 0.0) continue;

            hSumAngles->Fill(x, z, angle);
            hCounts->Fill(x, z);
        } catch (...) { continue; }
    }
    file.close();

    // --- IMAGE 1: Hit Density (Raw Counts) ---
    TCanvas *cCounts = new TCanvas("cCounts", "Hit Density", 800, 800);
    cCounts->SetLogz(); // <--- ADD THIS: Sets the color scale to Logarithmic
    hCounts->GetZaxis()->SetTitle("Number of Muons (Log Scale)");
    // Find the maximum and minimum bins to set a tight scale
double maxHits = hCounts->GetMaximum();
double minHits = hCounts->GetMinimum(0.01); // Get min value > 0

// Force the scale to focus only on the top 20-30% of the data
// where the shadow actually lives.
hCounts->GetZaxis()->SetRangeUser(minHits * 0.8, maxHits);
    hCounts->Draw("COLZ");
    cCounts->SaveAs("hit_density_map.png");

    // --- IMAGE 2: Angular Distribution (Average Angle) ---
    TCanvas *cAngles = new TCanvas("cAngles", "Angular Distribution", 800, 800);
    hSumAngles->Divide(hCounts); // Calculate Average
    hSumAngles->GetZaxis()->SetTitle("Average Angle (Radians)");
    hSumAngles->SetTitle("Muon Angular Distribution Map");
    hSumAngles->Draw("COLZ");
    cAngles->SaveAs("reconstruction_map.png");

    std::cout << "Success! Created both 'hit_density_map.png' and 'reconstruction_map.png'" << std::endl;
}
