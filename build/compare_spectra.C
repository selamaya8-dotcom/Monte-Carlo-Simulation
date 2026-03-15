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

    std::vector<std::pair<std::string, std::string>> pairs = {
        {"hDet1", "h_lecroy"},
        {"hDet2", "h2"}
    };

    for (auto const& p : pairs) {
        std::string simName = p.first;
        std::string expName = p.second;

        TH1F *hSim = (TH1F*)fSim->Get(simName.c_str());
        TH1D *hExp = (TH1D*)c1->GetPrimitive(expName.c_str());

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

        TH1D *hExpClone = (TH1D*)hExp->Clone(Form("%s_exp_norm", expName.c_str()));
        TH1D *hSimClone = (TH1D*)hSim->Clone(Form("%s_sim_norm", simName.c_str()));

        // Normalize
        hExpClone->Scale(1.0 / hExpClone->Integral());
        hSimClone->Scale(1.0 / hSimClone->Integral());

        hExpClone->SetMarkerStyle(20);
        hExpClone->SetMarkerSize(0.6);
        hSimClone->SetLineColor(kRed);
        hSimClone->SetLineWidth(2);

        TCanvas *cOut = new TCanvas(Form("c_%s", simName.c_str()), "Validation", 800, 900);
        auto rp = new TRatioPlot(hSimClone, hExpClone);
        rp->Draw();

        rp->GetUpperPad()->cd();

        // --- NEW STATISTICAL ANALYSIS ---
        // 1. Chi2 Test (NORM flag for normalized histograms)
        double chi2; int ndf; int igood;
        double chi2Prob = hExpClone->Chi2TestX(hSimClone, chi2, ndf, igood, "NORM");

        // 2. Shape Analysis (Mean and RMS)
        double meanDiff = hExpClone->GetMean() - hSimClone->GetMean();
        double rmsRatio = hExpClone->GetRMS() / hSimClone->GetRMS();

        // Update Legend with multiple metrics
        TLegend *leg = new TLegend(0.4, 0.65, 0.88, 0.88);
        leg->SetTextSize(0.025);
        leg->SetHeader(Form("Validation: %s", simName.c_str()));
        leg->AddEntry(hExpClone, "Experiment", "lep");
        leg->AddEntry(hSimClone, "Sim (Smeared)", "l");
        leg->AddEntry((TObject*)0, Form("Chi2/ndf: %.2f / %d", chi2, ndf), "");
        leg->AddEntry((TObject*)0, Form("Chi2 Prob: %.4f", chi2Prob), "");
        leg->AddEntry((TObject*)0, Form("Mean Delta: %.3f V", meanDiff), "");
        leg->AddEntry((TObject*)0, Form("RMS Ratio (Exp/Sim): %.3f", rmsRatio), "");
        leg->Draw();

        cOut->Update();
        cOut->SaveAs(Form("Comparison_%s.png", simName.c_str()));

        std::cout << "\nResults for " << simName << ":" << std::endl;
        std::cout << " - Chi2 Prob: " << chi2Prob << std::endl;
        std::cout << " - RMS Ratio: " << rmsRatio << " (Target 1.0)" << std::endl;
    }
}
