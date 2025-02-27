#!/bin/bash

#files=("samples-1000000.txt" "samples-10000000.txt" "samples-100000000.txt" "samples-1000000000.txt")
files=("samples-100.txt")

executable="./build/single_threaded/weather-stations/cle-ws"

num_runs=5

calculate_std_dev() {
    local times=("$@")
    local sum=0
    local sq_sum=0
    local count=${#times[@]}
    
    for t in "${times[@]}"; do
        sum=$((sum + t))
        sq_sum=$((sq_sum + t * t))
    done
    
    local mean=$((sum / count))
    local variance=$(((sq_sum / count) - mean * mean))
    local std_dev=$(echo "scale=2; sqrt($variance)" | bc)
    
    echo "$std_dev"
}

for f in "${files[@]}"; do
    echo "Running executable with file $f..."

    execution_times=()

    for ((i=1; i<=num_runs; i++)); do
        output=$($executable $f)

        exec_time=$(echo "$output" | grep -oP "Execution time: \K[0-9]+")
        
        if [ -n "$exec_time" ]; then
            execution_times+=("$exec_time")
            echo "Run $i: Execution time: $exec_time milliseconds"
        else
            echo "Error: Unable to parse execution time for run $i"
        fi
    done

    total=0
    min=999999999
    max=0

    for t in "${execution_times[@]}"; do
        total=$((total + t))
        if [ "$t" -lt "$min" ]; then
            min=$t
        fi
        if [ "$t" -gt "$max" ]; then
            max=$t
        fi
    done

    avg=$((total / num_runs))

    std_dev=$(calculate_std_dev "${execution_times[@]}")

    echo "Results for file $f:"
    echo "Average execution time: $avg milliseconds"
    echo "Minimum execution time: $min milliseconds"
    echo "Maximum execution time: $max milliseconds"
    echo "Standard deviation: $std_dev milliseconds"
    echo "----------------------------------------"
done
