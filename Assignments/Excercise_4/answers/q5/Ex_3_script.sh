#!/bin/bash

matrix=("Canny" "HoughLine" "HoughCircle")
resolutions=("640x480" "800x600" "1600x1200")
sched_policies=("SCHED_FIFO" "SCHED_OTHER")

rm example.csv
touch example.csv

# Iterate through each combination of parameters
for mat in "${matrix[@]}"; do
    for resolution in "${resolutions[@]}"; do
        for policy in "${sched_policies[@]}"; do
            echo "Running: ./program --transformation=${mat} --resolution=${resolution} --sched_policy=${policy}"
            sudo ./program --transformation=$mat --resolution=$resolution --sched_policy=$policy
            return_value="$?"
            # Check if the program exited with an error
            if [ "$return_value" -ne 0 ]; then
                echo "Error running program with parameters --transformation=${mat} --resolution=${resolution} --sched_policy=${policy}, exiting."
                exit -1
            fi
        done
    done
done
