#!/bin/bash

numbers=(1000000 10000000 100000000 1000000000)

executable="./build/samples/cle-samples"

start_time=$(date +%s)

for num in "${numbers[@]}"; do
    echo "Running executable with argument $num..."

    $executable $num

    echo "Generated files for argument $num"
done

end_time=$(date +%s)

execution_time=$((end_time - start_time))

echo "Total execution time: $execution_time seconds"
