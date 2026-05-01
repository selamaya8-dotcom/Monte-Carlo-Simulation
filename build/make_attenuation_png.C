#include <TCanvas.h>
#include <TH2D.h>
#include <TStyle.h>
#include <TColor.h>
#include <TPaveText.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

void make_attenuation_png(const char* no_sphere_file, const char* with_sphere_file) {
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kRainBow);
    gStyle->SetNumberContours(256);

    TH2D *hNo = new TH2D("hNo", ";X (cm);Z (cm)", 50, -55, 55, 50, -55, 55);
    TH2D *hYes = new TH2D("hYes", ";X (cm);Z (cm)", 50, -55, 55, 50, -55, 55);

    auto load_csv = [&](TH2D* h, const char* fname) -> int {
        std::ifstream f(fname);
        if (!f.is_open()) return 0;
        std::string line;
        std::getline(f, line);
        int cnt = 0;
        while (std::getline(f, line)) {
            std::stringstream ss(line);
            std::vector<double> row;
            std::string tok;
            while (std::getline(ss, tok, ',')) {
                try { row.push_back(std::stod(tok)); } catch (...) {}
            }
            if (row.size() >= 11) {
                double x = row[8]/10.0, z = row[10]/10.0;
                if (std::fabs(x) < 45 && std::fabs(z) < 45) {
                    h->Fill(x, z);
                    cnt++;
                }
            }
        }
        f.close();
        return cnt;
    };

    int n_no = load_csv(hNo, no_sphere_file);
    int n_yes = load_csv(hYes, with_sphere_file);

    std::cout << "No sphere: " << n_no << " events" << std::endl;
    std::cout << "With sphere: " << n_yes << " events" << std::endl;

    TH2D *hTrans = (TH2D*)hYes->Clone("Trans");
    hTrans->Divide(hNo);
    hTrans->SetMinimum(0.1);
    hTrans->SetMaximum(1.2);

    TCanvas *c1 = new TCanvas("c1", "No Sphere", 1000, 800);
    c1->SetLogz(); hNo->Draw("COLZ"); c1->SaveAs("01_no_sphere.png");

    TCanvas *c2 = new TCanvas("c2", "With Sphere", 1000, 800);
    c2->SetLogz(); hYes->Draw("COLZ"); c2->SaveAs("02_with_sphere.png");

    TCanvas *c3 = new TCanvas("c3", "Transmission", 1000, 800);
    hTrans->Draw("COLZ"); c3->SaveAs("03_transmission.png");

    TCanvas *c4 = new TCanvas("c4", "Summary", 1200, 600);
    c4->Divide(2,1);
    c4->cd(1); hNo->Draw("COLZ");
    c4->cd(2); hTrans->Draw("COLZ");
    c4->SaveAs("04_summary.png");

    std::cout << "\n PNGs created:" << std::endl;
    std::cout << "03_transmission.png ← (T<1 in center)" << std::endl;

    TCanvas *c5 = new TCanvas("c5", "Transmission T(X)", 1000, 600);
    c5->cd()->SetGrid();

    std::vector<double> x_vals, t_vals, t_errs;
    int nxbins = hTrans->GetNbinsX();

    for (int ix = 1; ix <= nxbins; ix++) {
        double x = hTrans->GetXaxis()->GetBinCenter(ix);
        double sum_t = 0, sum_w = 0, sum_n_no = 0;

        for (int iy = hTrans->GetYaxis()->FindBin(-10); iy <= hTrans->GetYaxis()->FindBin(10); iy++) {
            double t = hTrans->GetBinContent(ix, iy);
            double n_no = hNo->GetBinContent(ix, iy);
            double n_yes = hYes->GetBinContent(ix, iy);

            if (n_no > 5 && n_yes > 5) {
                double w = n_no * n_yes;
                sum_t += t * w;
                sum_w += w;
                sum_n_no += n_no;
            }
        }

        if (sum_w > 10) {  // Reliable statistics
            double t_avg = sum_t / sum_w;
            double err = 1.0 / sqrt(sum_n_no);
            x_vals.push_back(x);
            t_vals.push_back(t_avg);
            t_errs.push_back(err);
        }
    }

    int npoints = x_vals.size();
    TGraphErrors *graph = new TGraphErrors(npoints, &x_vals[0], &t_vals[0], 0, &t_errs[0]);
    graph->SetTitle("Transmission Function T(X);Generator X (cm);T(X)");
    graph->SetMarkerStyle(20);
    graph->SetMarkerSize(1.5);
    graph->SetMarkerColor(kBlue);
    graph->SetLineColor(kBlue);
    graph->SetLineWidth(3);
    graph->Draw("AP");  

    TF1 *baseline = new TF1("baseline", "1", -45, 45);
    baseline->SetLineColor(kGray);
    baseline->SetLineStyle(7);
    baseline->Draw("SAME");

    // Sphere edges
    /*
    TLine *edgeL = new TLine(-5, 0.5, -5, 1.2);
    TLine *edgeR = new TLine(5, 0.5, 5, 1.2);
    edgeL->SetLineColor(kRed); edgeL->SetLineStyle(2); edgeL->Draw();
    edgeR->SetLineColor(kRed); edgeR->SetLineStyle(2); edgeR->Draw();
*/
    TLegend *leg = new TLegend(0.65, 0.75, 0.9, 0.9);
    leg->AddEntry(graph, "T(X)", "lp");
    leg->AddEntry(baseline, "T=1.0 (no sphere)", "l");
    leg->Draw();

    c5->SaveAs("05_smooth_profile.png");

    std::cout << "Added: 05_smooth_profile.png" << std::endl;

}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: root -l -q make_attenuation_png.C+(\"file1.csv\",\"file2.csv\")" << std::endl;
        return 1;
    }
    make_attenuation_png(argv[1], argv[2]);
    return 0;
}
