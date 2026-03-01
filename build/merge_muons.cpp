#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cmath>
using namespace std;

struct EventData {
    double totalEnergy = 0;
    double xA = 0, zA = 0; // Position on first detector
    double pxA = 0, pyA = 0, pzA = 0; // Direction into sphere
    double pxB = 0, pyB = 0, pzB = 0; // Direction out of sphere
    bool hasA = false;
    bool hasB = false;
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
        // ... inside while(getline(fin, line)) ...
        string label, eid, x_s, y_s, z_s, px_s, py_s, pz_s, edep_s;

        getline(ss, label, ',');
        getline(ss, eid, ',');
        getline(ss, x_s, ',');
        getline(ss, y_s, ',');
        getline(ss, z_s, ',');
        getline(ss, px_s, ',');
        getline(ss, py_s, ',');
        getline(ss, pz_s, ',');
        getline(ss, edep_s, ','); // Read the 9th column

        int eventID = stoi(eid);
        auto& data = events[eventID];
        data.totalEnergy += stod(edep_s); // Accumulate energy for the whole event

        if (label == "A") {
            data.xA = stod(x_s);
            data.zA = stod(z_s);
            data.pxA = stod(px_s);
            data.pyA = stod(py_s);
            data.pzA = stod(pz_s);
            data.hasA = true;
        } else if (label == "B") {
            data.pxB = stod(px_s);
            data.pyB = stod(py_s);
            data.pzB = stod(pz_s);
            data.hasB = true;
        }
    }

    fin.close();

    // ... inside main() in merge_muons.cpp ...

    ofstream fout(outputFileName);
    fout << "EventID,xA,zA,ScatteringAngle,TotalEnergy\n"; // Header

    for (auto& [id, data] : events) {
        if (data.hasA && data.hasB) {
            // 1. Calculate Angle (keep your existing logic)
            double magA = sqrt(data.pxA*data.pxA + data.pyA*data.pyA + data.pzA*data.pzA);
            double magB = sqrt(data.pxB*data.pxB + data.pyB*data.pyB + data.pzB*data.pzB);
            double dot = (data.pxA*data.pxB + data.pyA*data.pyB + data.pzA*data.pzB) / (magA * magB);
            if (dot > 1.0) dot = 1.0;
            if (dot < -1.0) dot = -1.0;
            double angle = acos(dot);

            // 2. PROJECT TO SPHERE HEIGHT
            // From your code: sphereY is roughly 1875mm, detectorY is roughly 635mm
            double targetY = 1875.0;
            double detectorY = 635.0;
            double deltaY = targetY - detectorY;

            // Use the slope of the momentum (px/py and pz/py) to find X and Z at targetY
            // Note: py is negative because muons are going DOWN
            double x_projected = data.xA + (data.pxA / abs(data.pyA)) * deltaY;
            double z_projected = data.zA + (data.pzA / abs(data.pyA)) * deltaY;

            // 3. Write PROJECTED coordinates to CSV
            fout << id << "," << x_projected << "," << z_projected << ","
                 << angle << "," << data.totalEnergy << "\n";
        }
    }
    fout.close();
    cout << "Success: Merged into " << outputFileName << endl;
    return 0;
}
