#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct EventData {
    double totalEnergy = 0.0;

    double energyA = 0.0;
    double energyB = 0.0;

    double xA = 0.0;
    double yA = 0.0;
    double zA = 0.0;

    double genAngle = 0.0;
    double genEnergy = 0.0;
    double genPosX = 0.0;
    double genPosY = 0.0;
    double genPosZ = 0.0;

    double momXB = 0.0;
    double momYB = 0.0;
    double momZB = 0.0;

    bool hasA = false;
    bool hasB = false;
};

int main() {

    const char* env_id = std::getenv("G4_RUN_ID");
    const double radToDeg = 180.0 / M_PI;
    string id_str = (env_id != nullptr) ? string(env_id) : "";

    const string inputFileName =
        id_str.empty() ? "hits_output.csv" : "hits_output_" + id_str + ".csv";
    const string outputFileName =
        id_str.empty() ? "combined_hits.csv" : "combined_hits_" + id_str + ".csv";
    const string spectrumFileName =
        id_str.empty() ? "spectrum_data.csv" : "spectrum_data_" + id_str + ".csv";

    ifstream fin(inputFileName);
    if (!fin.is_open()) {
        cerr << "Error: Could not open " << inputFileName << endl;
        return 1;
    }

    map<int, EventData> events;
    string line;

    getline(fin, line);

    while (getline(fin, line)) {
        if (line.empty() || line.find("Detector") != string::npos) {
            continue;
        }

        stringstream ss(line);
        string token;
        vector<string> columns;

        while (getline(ss, token, ',')) {
            columns.push_back(token);
        }

        if (columns.size() < 14) {
            continue;
        }

        try {
            const int eventID = stoi(columns[1]);
            EventData& data = events[eventID];

            const double edep = stod(columns[8]);

            data.totalEnergy += edep;
            data.genAngle = stod(columns[9]);
            data.genEnergy = stod(columns[10]);
            data.genPosX = stod(columns[11]);
            data.genPosY = stod(columns[12]);
            data.genPosZ = stod(columns[13]);

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
        }
        catch (...) {
            continue;
        }
    }

    fin.close();

    ofstream fout(outputFileName);
    ofstream fspec(spectrumFileName);

    fout << "EventID,PosX,PosY,PosZ,ScatteringAngle,TotalEnergy,GenAngle,GenEnergy,GenPosX,GenPosY,GenPosZ\n";
    fspec << "EventID,EnergyA,EnergyB\n";

    int count = 0;

    for (const auto& item : events) {
        const int id = item.first;
        const EventData& data = item.second;

        if (data.energyA >= 1.0 && data.energyB >= 1.0) {
            fspec << id << "," << data.energyA << "," << data.energyB << "\n";

            const double finalTheta = acos(abs(data.momYB));
            const double scattering = abs(finalTheta - data.genAngle);

            fout << id << ","
                 << data.xA << "," << data.yA << "," << data.zA << ","
                 << fixed << setprecision(8) << scattering << ","
                 << data.totalEnergy << ","
                 << data.genAngle * radToDeg << ","
                 << data.genEnergy << ","
                 << data.genPosX << "," << data.genPosY << "," << data.genPosZ << "\n";

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
