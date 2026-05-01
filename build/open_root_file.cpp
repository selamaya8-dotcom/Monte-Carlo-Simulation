void open_root_file(const char* fileName) {

    TFile *f = TFile::Open(fileName, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error opening ROOT file!" << std::endl;
        return;
    }

    TH1F *h = (TH1F*)f->Get("hEnergyDeposit");
    if (!h) {
        std::cerr << "Histogram 'hEnergyDeposit' not found in file!" << std::endl;
        f->Close();
        return;
    }

    h->Draw();

    int num_events = h->Integral(h->FindBin(60),h->FindBin(200));

    cout << "Number of Events above 60 MeV = " << num_events << endl;

}
