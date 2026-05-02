#! /usr/bin/env bash

echo "Executing benchmark and saving results..."

BENCHMARK=build/benchmark
if [ ! -f $BENCHMARK ]; then
    echo "benchmark binary does not exist"
    exit
fi

function execute_uint64_100M() {
    echo "Executing operations for $1 and index $2"
    echo "Executing insert+lookup mixed workload with insert-ratio 0.9"
    $BENCHMARK ./data/$1 ./data/$1_ops_2M_0.000000rq_0.500000nl_0.900000i_0m_mix --through --csv --only $2 -r 3 # benchmark insert and lookup mix
    echo "Executing insert+lookup mixed workload with insert-ratio 0.1"
    $BENCHMARK ./data/$1 ./data/$1_ops_2M_0.000000rq_0.500000nl_0.100000i_0m_mix --through --csv --only $2 -r 3 # benchmark insert and lookup mix
}

mkdir -p ./results

DATA=fb_100M_public_uint64
for INDEX in HybridPGMLIPP # for INDEX in LIPP DynamicPGM HybridPGMLIPP
do
    execute_uint64_100M ${DATA} $INDEX
done

echo "===================Benchmarking complete!===================="

# add header for csv files
for FILE in ./results/*.csv
do
    # Run the mixed workloads
    if [[ $FILE == *mix* ]]; then
        # For insert+lookup mixed workload
        # Remove existing header if present
        if head -n 1 $FILE | grep -q "index_name"; then
            sed -i '1d' $FILE  # Delete the first line
        fi
        # Add the header
        sed -i '1s/^/index_name,build_time_ns1,build_time_ns2,build_time_ns3,index_size_bytes,mixed_throughput_mops1,mixed_throughput_mops2,mixed_throughput_mops3,search_method,value,percent\n/' $FILE
        echo "Header set for $FILE"
    fi
done
