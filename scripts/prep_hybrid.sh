#!/bin/bash
# Run all steps to prepare the hybrid benchmark

set -e  # Exit on error

echo "=== Preparing Benchmark ==="

# Make scripts executable
chmod +x scripts/*.sh

echo "Step 1: Downloading dataset..."
./scripts/download_dataset.sh

echo "Step 2: Creating minimal CMakeLists.txt..."
./scripts/create_minimal_cmake.sh

echo "Step 3: Generating lookup workloads..."
./scripts/generate_hybrid_workloads.sh

echo "Step 4: Building minimal benchmark..."
./scripts/build_benchmark.sh