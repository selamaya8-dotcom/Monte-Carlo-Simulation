#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cmath>

using namespace std;

struct EventData {
    double totalEnergy = 0;
    double energyA = 0;
    double energyB = 0;
    double xA = 0, yA = 0, zA = 0;
    double genAngle = 0;
    double momXB = 0, momYB = 0, momZB = 0;
    bool hasA = false;
    bool hasB = false;
    double genEnergy = 0;
};

int main() {
    const char* env_id = std::getenv("G4_RUN_ID");
    const double radToDeg = 180.0 / M_PI;
    string id_str = (env_id) ? std::string(env_id) : "";

    string inputFileName = id_str.empty() ? "hits_output.csv" : "hits_output_" + id_str + ".csv";
    string outputFileName = id_str.empty() ? "combined_hits.csv" : "combined_hits_" + id_str + ".csv";
    string spectrumFileName = id_str.empty() ? "spectrum_data.csv" : "spectrum_data_" + id_str + ".csv";

    ifstream fin(inputFileName);
    if (!fin.is_open()) {
        cerr << "Error: Could not open " << inputFileName << endl;
        return 1;
    }

    map<int, EventData> events;
    string line;
    getline(fin, line); // Skip header

    while (getline(fin, line)) {
        if (line.empty() || line.find("Detector") != string::npos) continue;

        stringstream ss(line);
        string token;
        vector<string> columns;

        while (getline(ss, token, ',')) {
            columns.push_back(token);
        }

        // We now expect 10 columns:
        // 0:Det, 1:EID, 2:X, 3:Y, 4:Z, 5:Mx, 6:My, 7:Mz, 8:Edep, 9:GenAngle
        if (columns.size() < 11) continue;

        try {
            int eventID = stoi(columns[1]);
            auto& data = events[eventID];

            // Always update energy and GenAngle for every hit in the event
            double edep = stod(columns[8]);
            data.totalEnergy += stod(columns[8]);
            data.genAngle = stod(columns[9]);
            data.genEnergy = stod(columns[10]);

            if (columns[0] == "A") {
                data.xA = stod(columns[2]);
                data.yA = stod(columns[3]);
                data.zA = stod(columns[4]);
                data.energyA += edep;
                data.hasA = true;
            }
            else if (columns[0] == "B") {
                data.momXB = stod(columns[5]);
                data.momYB = stod(columns[6]);
                data.momZB = stod(columns[7]);
                data.energyB += edep;
                data.hasB = true;
            }
        } catch (...) { continue; }
    }
    fin.close();


    ofstream fout(outputFileName);
    ofstream fspec(spectrumFileName);

    fout << "EventID,PosX,PosY,PosZ,ScatteringAngle,TotalEnergy,GenAngle,GenEnergy\n";
    fspec << "EventID,EnergyA,EnergyB\n";

    int count = 0;
    for (auto const& item : events) {
        int id = item.first;
        const EventData& data = item.second;

        // Logic Change: Check if BOTH detectors have energy >= 1.0 MeV
        if (data.energyA >= 1.0 && data.energyB >= 1.0) {

            fspec << id << "," << data.energyA << "," << data.energyB << "\n";

            // 2. Perform physics calculations
            double finalTheta = std::acos(std::abs(data.momYB));
            double genAngleDeg = data.genAngle * radToDeg;
            double genEnergy = data.genEnergy;
            double scattering = std::abs(finalTheta - data.genAngle);

            // 3. Save to combined hits file
            fout << id << ","
                 << data.xA << "," << data.yA << "," << data.zA << ","
                 << fixed << setprecision(8) << scattering << ","
                 << data.totalEnergy << ","
                 << genAngleDeg << ","
                 << genEnergy << "\n";

            count++;
        }
    }
    fout.close();
    fspec.close();
    cout << "Processed " << events.size() << " unique events." << endl;
    cout << "Saved " << count << " physics events to " << outputFileName << endl;
    cout << "Saved spectrum data to " << spectrumFileName << endl;

    return 0;
}
