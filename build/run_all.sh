#!/bin/bash

# Check if correct number of arguments are provided
if [ $# -ne 2 ]; then
  echo "Usage: $0 <density> <number_of_runs>"
  exit 1
fi

density=$1
runs=$2

# Use the Process ID ($$) to make filenames unique for this instance
instance_id=$$
master_file="accumulated_${density}_pid${instance_id}.csv"
# Ensure the executables are compiled and ready
make

echo "Starting $runs runs for density $density..."

for (( i=1; i<=$runs; i++ ))
do
  unique_run_id="${instance_id}_${i}"
  export G4_RUN_ID=$unique_run_id

  # Create a temporary macro for this specific run
  tmp_macro="temp_${instance_id}_run_${i}.mac"
  echo "/soil/density ${density} g/cm3" > $tmp_macro
  echo "/run/initialize" >> $tmp_macro
  echo "/random/setSeeds $RANDOM $RANDOM" >> $tmp_macro
  echo "/run/beamOn 10000" >> $tmp_macro

  # 1. Run Geant4 simulation
  ./DroneMuonScan $tmp_macro

  # 2. Run Merge script (Produces combined_hits_ID.csv)
  ./merge_muons

  # 3. Append data to the Master CSV
  if [ "$i" -eq 1 ]; then
    # For the first run, keep the header
    cat "combined_hits_${unique_run_id}.csv" > "$master_file"
  else
    # For subsequent runs, skip the header to keep the CSV clean
    tail -n +2 "combined_hits_${unique_run_id}.csv" >> "$master_file"
  fi

  # Cleanup individual run files to save disk space
  rm $tmp_macro "hits_output_${unique_run_id}.csv" "combined_hits_${unique_run_id}.csv"
done

echo "${master_file} was created succesufully"
