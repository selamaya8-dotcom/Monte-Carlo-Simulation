#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: $0 <density> <number_of_runs>"
  exit 1
fi

density=$1
runs=$2

# Process ID to make filenames unique for instance
instance_id=$$
master_file="accumulated_${density}_pid${instance_id}.csv"
# Master file for spectrum data
spectrum_master="spectrum_accumulated_${density}_pid${instance_id}.csv"

echo "Starting $runs runs for density $density..."

for (( i=1; i<=$runs; i++ ))
do
  unique_run_id="${instance_id}_${i}"
  export G4_RUN_ID=$unique_run_id

  tmp_macro="temp_${instance_id}_run_${i}.mac"
  echo "/soil/density ${density} g/cm3" > $tmp_macro
  echo "/run/initialize" >> $tmp_macro
  echo "/random/setSeeds $RANDOM $RANDOM" >> $tmp_macro
  echo "/run/beamOn 10000" >> $tmp_macro

  ./DroneMuonScan $tmp_macro

  ./merge_muons

  if [ "$i" -eq 1 ]; then
    cat "combined_hits_${unique_run_id}.csv" > "$master_file"
    cat "spectrum_data_${unique_run_id}.csv" > "$spectrum_master"
  else
    tail -n +2 "combined_hits_${unique_run_id}.csv" >> "$master_file"
    tail -n +2 "spectrum_data_${unique_run_id}.csv" >> "$spectrum_master"
  fi

  rm $tmp_macro "hits_output_${unique_run_id}.csv" "combined_hits_${unique_run_id}.csv" "spectrum_data_${unique_run_id}.csv"
done

echo "${master_file} was created successfully"
echo "${spectrum_master} was created successfully"
