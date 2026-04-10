#include "benchmarks/benchmark_hybrid_pgm_lipp.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/hybrid_pgm_lipp.h"

template <typename Searcher>
void benchmark_64_hybrid_pgm_lipp(tli::Benchmark<uint64_t>& benchmark, 
                              bool pareto, const std::vector<int>& params) {
  if (!pareto){
    util::fail("Hybrid PGM LIPP's hyperparameter cannot be set");
  }
  else {
    // Sweep pgm_error with flush_ratio=2
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 16, 2>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 32, 2>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 64, 2>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 128, 2>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 256, 2>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 512, 2>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 1024, 2>>();
    
    // Sweep pgm_error with flush_ratio=5
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 16, 5>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 32, 5>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 64, 5>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 128, 5>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 256, 5>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 512, 5>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 1024, 5>>();
    
    // Sweep pgm_error with flush_ratio=10
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 16, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 32, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 64, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 128, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 256, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 512, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 1024, 10>>();
  }
}

template <int record>
void benchmark_64_hybrid_pgm_lipp(tli::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("fb_100M") != std::string::npos) {
    if (filename.find("0.000000i") != std::string::npos) {
      benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 2>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 2>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 16, 2>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 5>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 5>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 16, 5>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 10>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 10>>();
      benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 16, 10>>();
    } else if (filename.find("mix") == std::string::npos) {
      if (filename.find("0m") != std::string::npos) {
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 256, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 256, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 512, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 256, 10>>();
      } else if (filename.find("1m") != std::string::npos) {
        benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 1024, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, InterpolationSearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 256, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 1024, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, InterpolationSearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 256, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 1024, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, InterpolationSearch<record>, 512, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 256, 10>>();
      } else if (filename.find("2m") != std::string::npos) {
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, InterpolationSearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, InterpolationSearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, InterpolationSearch<record>, 512, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 512, 10>>();
      }
    } else {
      if (filename.find("0.050000i") != std::string::npos) {
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 10>>();
      } else if (filename.find("0.500000i") != std::string::npos) {
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 10>>();
      } else if (filename.find("0.900000i") != std::string::npos) {
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 10>>();
      } else if (filename.find("0.100000i") != std::string::npos) {
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 2>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 5>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 512, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 10>>();
        benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 10>>();
      }
    }
  }
}

INSTANTIATE_TEMPLATES_MULTITHREAD(benchmark_64_hybrid_pgm_lipp, uint64_t);