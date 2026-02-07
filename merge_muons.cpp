#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
using namespace std;

struct EventData {
    double totalEnergy = 0;
    double xSum = 0;
    double ySum = 0;
    int count = 0;
};

int main(int argc, char* argv[]) {
    // 1. Get the ID from the environment, exactly like your EventAction.cpp
    const char* env_id = std::getenv("G4_RUN_ID");
    string id_str = (env_id) ? std::string(env_id) : "";

    // 2. Set filenames based on the ID
    string inputFileName = id_str.empty() ? "hits_output.csv" : "hits_output_" + id_str + ".csv";
    string outputFileName = id_str.empty() ? "combined_hits.csv" : "combined_hits_" + id_str + ".csv";

    ifstream fin(inputFileName);
    if (!fin) {
        cerr << "Cannot open file: " << inputFileName << "\n";
        return 1;
    }
    string line;
    map<int, EventData> events;

    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);

        string det_field, event_energy_field, coords_field;

        if (!getline(ss, det_field, ',')) continue;
        if (!getline(ss, event_energy_field, ',')) continue;
        if (!getline(ss, coords_field)) continue;

        if (det_field.empty() || event_energy_field.empty() || coords_field.empty())
            continue;

        // Detector (first character of det_field)
        char detector = det_field[0];

        // Event ID and Energy deposition parsing
        // event_energy_field looks like "11: 4.17858"
        int eventID;
        double energy;
        {
            size_t colon_pos = event_energy_field.find(':');
            if (colon_pos == string::npos) continue;
            string event_id_str = event_energy_field.substr(0, colon_pos);
            string energy_str = event_energy_field.substr(colon_pos + 1);

            try {
                eventID = stoi(event_id_str);
                energy = stod(energy_str);
            } catch (...) {
                continue; // parsing error
            }
        }

        // Coordinates parsing: coords_field looks like "(53.757,30.8595)"
        double x, y;
        {
            size_t start = coords_field.find('(');
            size_t comma = coords_field.find(',', start);
            size_t end = coords_field.find(')', comma);
            if (start == string::npos || comma == string::npos || end == string::npos)
                continue;
            string x_str = coords_field.substr(start + 1, comma - start - 1);
            string y_str = coords_field.substr(comma + 1, end - comma - 1);
            try {
                x = stod(x_str);
                y = stod(y_str);
            } catch (...) {
                continue;
            }
        }

        // Aggregate event data
        events[eventID].totalEnergy += energy;
        events[eventID].xSum += x;
        events[eventID].ySum += y;
        events[eventID].count++;
    }

    fin.close();

    ofstream fout(outputFileName);
    fout << "EventID,TotalEnergy,Xavg,Yavg\n";
    for (auto& [id, data] : events) {
        //if (data.count == 1) {  // only events with hits in both detectors
            fout << id << "," << data.totalEnergy << ","
                 << data.xSum / data.count << "," << data.ySum / data.count << "\n";
        //}
    }
    fout.close();
    cout << "Success: Merged into " << outputFileName << endl;
    return 0;
}
