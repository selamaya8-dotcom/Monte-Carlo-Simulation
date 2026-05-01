#include <TFile.h>
#include <TH1D.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <THStack.h>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

void compare_spectra() {
    gStyle->SetOptStat(0);

    TFile *fExp = TFile::Open("Spectrum.root");
    TFile *fSim = TFile::Open("detector_spectrum.root");

    if (!fExp || !fSim) return;

    TCanvas *c1 = (TCanvas*)fExp->Get("c1");
    if (!c1) return;

    std::vector<std::pair<std::string, std::string>> pairs = {
        {"hDet1", "h2"},
        {"hDet2", "h_lecroy"}
    };

    for (const auto& p : pairs) {
        TH1F *hSim = (TH1F*)fSim->Get(p.first.c_str());
        TH1D *hExp = (TH1D*)c1->GetPrimitive(p.second.c_str());

        if (!hExp) {
            TIter next(c1->GetListOfPrimitives());
            TObject *obj;
            while ((obj = next())) {
                if (obj->InheritsFrom(THStack::Class())) {
                    hExp = (TH1D*)((THStack*)obj)->GetHists()->FindObject(p.second.c_str());
                    if (hExp) break;
                }
            }
        }

        if (!hExp || !hSim) continue;

        double mExp = hExp->GetMean();
        double mSim = hSim->GetMean();
        double rExp = hExp->GetRMS();
        double rSim = hSim->GetRMS();

        TH1D *hEN = (TH1D*)hExp->Clone("hEN");
        TH1D *hSN = (TH1D*)hSim->Clone("hSN");
        hEN->Scale(1.0 / hEN->Integral());
        hSN->Scale(1.0 / hSN->Integral());

        TCanvas *cOut = new TCanvas(Form("c_%s", p.first.c_str()), "Compare", 800, 600);

        hEN->SetLineColor(kBlack);
        hEN->SetLineWidth(1);
        hEN->SetFillColor(kBlack);
        hEN->SetTitle(Form("Detector Comparison: %s", p.first.c_str()));
        hEN->Draw("HIST"); 

        hSN->SetLineColor(p.first == "hDet1" ? kBlue : kRed);
        hSN->SetFillColorAlpha(p.first == "hDet1" ? kBlue : kRed, 0.2); 
        hSN->Draw("HIST SAME");

        double maxVal = hSN->GetMaximum();
        hEN->SetMaximum(maxVal * 1.1);

        TLegend *leg = new TLegend(0.58, 0.65, 0.88, 0.88);
        leg->AddEntry(hEN, "Experiment", "l");
        leg->AddEntry(hSN, "Simulation", "f");
        leg->AddEntry((TObject*)0, Form("Mean Ratio: %.3f", mExp / mSim), "");
        leg->AddEntry((TObject*)0, Form("RMS Ratio:  %.3f", rExp / rSim), "");
        leg->Draw();

        cOut->Update();
        cOut->SaveAs(Form("Comparison_%s.png", p.first.c_str()));

        cOut->Update();
    }
}
