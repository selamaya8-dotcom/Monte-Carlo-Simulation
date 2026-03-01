#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: $0 <density> <number_of_runs>"
  exit 1
fi
density=$1
runs=$2
# Use the Process ID ($$) to make filenames unique for this specific instance
instance_id=$$
master_file="accumulated_${density}_pid${instance_id}.csv"

# Optional: Ensure the executable is ready before starting parallel runs
make

for (( i=1; i<=$runs; i++ ))
do
  unique_run_id="${instance_id}_${i}"
  export G4_RUN_ID=$unique_run_id

  tmp_macro="temp_${instance_id}_run_${i}.mac"
  echo "/soil/density ${density} g/cm3" > $tmp_macro
  echo "/run/initialize" >> $tmp_macro
  echo "/random/setSeeds $RANDOM $RANDOM" >> $tmp_macro
  echo "/run/beamOn 10000" >> $tmp_macro

  # 1. Run Geant4 (Creates hits_output_ID.csv)
  ./DroneMuonScan $tmp_macro

  # 2. Run Merge (Creates combined_hits_ID.csv)
  ./merge_muons

  # 3. Append to Master (Accumulate)
  # Use 'tail -n +2' to skip the CSV header for every run after the first
  if [ "$i" -eq 1 ]; then
    cat "combined_hits_${unique_run_id}.csv" > "$master_file"
  else
    tail -n +2 "combined_hits_${unique_run_id}.csv" >> "$master_file"
  fi

  # Cleanup individual run files to save space
  rm $tmp_macro "hits_output_${unique_run_id}.csv" "combined_hits_${unique_run_id}.csv"
done

# Generate a unique histogram for this specific batch
./make_histogram "$master_file" "histogram_${density}_pid${instance_id}.root"
echo "Instance $instance_id finished. Output: $master_file"
