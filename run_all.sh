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
  # Unique macro name so parallel runs don't overwrite each other's settings
  tmp_macro="temp_${instance_id}_run_${i}.mac"
  echo "/soil/density ${density} g/cm3" > $tmp_macro
  echo "/run/initialize" >> $tmp_macro
  # IMPORTANT: Use a random seed so parallel runs aren't identical
  echo "/random/setSeeds $RANDOM $RANDOM" >> $tmp_macro
  echo "/run/beamOn 10000" >> $tmp_macro
  # Run Simulation
  ./DroneMuonScan $tmp_macro
  ./merge_muons "hits_output_${unique_run_id}.csv"

  cat "combined_hits_${unique_run_id}.csv" >> "$master_file"
  rm $tmp_macro "hits_output_${unique_run_id}.csv" "combined_hits_${unique_run_id}.csv"
done

# Generate a unique histogram for this specific batch
./make_histogram "$master_file" "histogram_${density}_pid${instance_id}.root"
echo "Instance $instance_id finished. Output: $master_file"
