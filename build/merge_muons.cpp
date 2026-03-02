#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

struct EventData {
    double totalEnergy = 0;
    double xA = 0, yA = 0, zA = 0;
    double genAngle = 0;
    bool hasA = false;
    bool hasB = false;
};

int main() {
    const char* env_id = std::getenv("G4_RUN_ID");
    string id_str = (env_id) ? std::string(env_id) : "";

    string inputFileName = id_str.empty() ? "hits_output.csv" : "hits_output_" + id_str + ".csv";
    string outputFileName = id_str.empty() ? "combined_hits.csv" : "combined_hits_" + id_str + ".csv";

    ifstream fin(inputFileName);
    if (!fin.is_open()) {
        cerr << "Error: Could not open " << inputFileName << endl;
        return 1;
    }

    map<int, EventData> events;
    string line;
    getline(fin, line); // Skip header

    while (getline(fin, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string token;
        vector<string> columns;

        while (getline(ss, token, ',')) {
            // Remove potential carriage returns from Windows-style line endings
            token.erase(remove(token.begin(), token.end(), '\r'), token.end());
            token.erase(remove(token.begin(), token.end(), '\n'), token.end());
            columns.push_back(token);
        }

        // We need at least 7 columns
        if (columns.size() < 7) continue;

        try {
            int eventID = stoi(columns[1]);
            auto& data = events[eventID];

            data.totalEnergy += stod(columns[5]); // Edep
            data.genAngle = stod(columns[6]);    // GenAngle

            if (columns[0] == "A") {
                data.xA = stod(columns[2]);
                data.yA = stod(columns[3]);
                data.zA = stod(columns[4]);
                data.hasA = true;
            } else if (columns[0] == "B") {
                // We keep the angle and energy, but A's position is usually the reference
                data.hasB = true;
            }
        } catch (const std::exception& e) {
            // If you see this, one of the columns isn't a number
            continue;
        }
    }
    fin.close();

    ofstream fout(outputFileName);
    fout << "EventID,PosX,PosY,PosZ,GenAngle,TotalEnergy\n";

    int count = 0;
    for (auto const& [id, data] : events) {
        // Changed to || to catch single hits too
        if (data.hasA || data.hasB) {
            fout << id << ","
                 << data.xA << "," << data.yA << "," << data.zA << ","
                 << fixed << setprecision(8) << data.genAngle << ","
                 << data.totalEnergy << "\n";
            count++;
        }
    }
    fout.close();

    cout << "Processed " << events.size() << " unique events." << endl;
    cout << "Saved " << count << " events to " << outputFileName << endl;

    return 0;
}
