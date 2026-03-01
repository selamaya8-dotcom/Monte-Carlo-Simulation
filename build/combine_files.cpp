#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

int main() {
    std::string prefix = "accumulated_2.0_pid";
    std::string output_filename = "final_accumulated.csv";

    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not create output file." << std::endl;
        return 1;
    }

    bool header_written = false;
    int files_processed = 0;

    std::cout << "Starting consolidation..." << std::endl;

    // Iterate through files in the current directory
    for (const auto& entry : fs::directory_iterator(fs::current_path())) {
        std::string filename = entry.path().filename().string();

        // Check if file starts with the specified prefix
        if (filename.rfind(prefix, 0) == 0) {
            std::ifstream infile(entry.path());
            if (!infile.is_open()) {
                std::cerr << "Could not open: " << filename << std::endl;
                continue;
            }

            std::string line;
            bool is_first_line = true;

            while (std::getline(infile, line)) {
                // Handle the header logic
                if (is_first_line) {
                    if (!header_written) {
                        outfile << line << "\n";
                        header_written = true;
                    }
                    is_first_line = false;
                    continue; // Skip the header for all subsequent files
                }

                // Write data line if it's not empty
                if (!line.empty()) {
                    outfile << line << "\n";
                }
            }

            std::cout << "Processed: " << filename << std::endl;
            files_processed++;
            infile.close();
        }
    }

    outfile.close();

    if (files_processed > 0) {
        std::cout << "Success! Created '" << output_filename
                  << "' using " << files_processed << " files." << std::endl;
    } else {
        std::cout << "No files found matching prefix: " << prefix << std::endl;
    }

    return 0;
}
