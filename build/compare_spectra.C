#include <TFile.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <THStack.h>
#include <TRatioPlot.h>
#include <iostream>
#include <vector>

void compare_spectra() {
    gStyle->SetOptStat(0);

    TFile *fExp = TFile::Open("Spectrum.root");
    TFile *fSim = TFile::Open("detector_spectrum.root");

    if (!fExp || fExp->IsZombie() || !fSim || fSim->IsZombie()) {
        std::cerr << "Error: Check if Spectrum.root and detector_spectrum.root exist." << std::endl;
        return;
    }

    TCanvas *c1 = (TCanvas*)fExp->Get("c1");
    if (!c1) return;

    // Define the pairing logic
    std::vector<std::pair<std::string, std::string>> pairs = {
        {"hDet1", "h_lecroy"},
        {"hDet2", "h2"}
    };

    for (auto const& p : pairs) {
        std::string simName = p.first;
        std::string expName = p.second;

        TH1F *hSim = (TH1F*)fSim->Get(simName.c_str());
        TH1D *hExp = (TH1D*)c1->GetPrimitive(expName.c_str());

        // Search stack if direct primitive fails
        if (!hExp) {
            TIter next(c1->GetListOfPrimitives());
            TObject *obj;
            while ((obj = next())) {
                if (obj->InheritsFrom(THStack::Class())) {
                    hExp = (TH1D*)((THStack*)obj)->GetHists()->FindObject(expName.c_str());
                    if (hExp) break;
                }
            }
        }

        if (!hExp || !hSim) continue;

        // Clone and Normalize (Area = 1)
        TH1D *hExpClone = (TH1D*)hExp->Clone(Form("%s_exp_norm", expName.c_str()));
        TH1F *hSimClone = (TH1F*)hSim->Clone(Form("%s_sim_norm", simName.c_str()));
        hExpClone->Scale(1.0 / hExpClone->Integral());
        hSimClone->Scale(1.0 / hSimClone->Integral());

        // Visual Styling
        hExpClone->SetMarkerStyle(20);
        hExpClone->SetMarkerSize(0.6);
        hSimClone->SetLineColor(kRed);
        hSimClone->SetLineWidth(2);

        // Canvas with Ratio Plot
        TCanvas *cOut = new TCanvas(Form("c_%s", simName.c_str()), "Validation", 800, 900);
        auto rp = new TRatioPlot(hSimClone, hExpClone);
        rp->Draw();

        // Legend and KS Test
        rp->GetUpperPad()->cd();
        double ksProb = hExpClone->KolmogorovTest(hSimClone);

        TLegend *leg = new TLegend(0.45, 0.7, 0.88, 0.88);
        leg->SetHeader(Form("Comparison: %s", simName.c_str()));
        leg->AddEntry(hExpClone, "Experimental (Data)", "lep");
        leg->AddEntry(hSimClone, "Simulation (Smeared)", "l");
        leg->AddEntry((TObject*)0, Form("KS Probability: %.4f", ksProb), "");
        leg->Draw();

        cOut->Update();
        cOut->SaveAs(Form("Comparison_%s.png", simName.c_str()));

        std::cout << "Generated: Comparison_" << simName << ".png (KS: " << ksProb << ")" << std::endl;
    }
}
