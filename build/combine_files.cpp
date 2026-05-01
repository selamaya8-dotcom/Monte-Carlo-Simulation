#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

void consolidate(const std::string& prefix, const std::string& output_filename) {
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not create output file: " << output_filename << std::endl;
        return;
    }

    bool header_written = false;
    int files_processed = 0;

    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        std::string filename = entry.path().filename().string();

        if (filename.rfind(prefix, 0) == 0) {
            std::ifstream infile(entry.path());
            if (!infile.is_open()) {
                std::cerr << "Could not open: " << filename << std::endl;
                continue;
            }

            std::string line;
            bool is_first_line = true;

            while (std::getline(infile, line)) {
                if (is_first_line) {
                    if (!header_written) {
                        outfile << line << "\n";
                        header_written = true;
                    }
                    is_first_line = false;
                    continue;
                }

                if (!line.empty()) {
                    outfile << line << "\n";
                }
            }

            files_processed++;
            infile.close();
        }
    }

    outfile.close();

    if (files_processed > 0) {
        std::cout << "Successfully consolidated " << files_processed
                  << " files into '" << output_filename << "'" << std::endl;
    } else {
        std::cout << "No files found for prefix: " << prefix << std::endl;
    }
}

int main() {
    std::string phys_prefix = "accumulated_"; // Catches any density
    std::string phys_output = "final_accumulated_physics.csv";

    std::string spec_prefix = "spectrum_accumulated_";
    std::string spec_output = "final_accumulated_spectrum.csv";

    std::cout << "Starting final consolidation..." << std::endl;

    consolidate(phys_prefix, phys_output);
    consolidate(spec_prefix, spec_output);

    return 0;
}
