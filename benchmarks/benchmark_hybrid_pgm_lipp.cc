#include "benchmarks/benchmark_hybrid_pgm_lipp.h"

#include "benchmark.h"
#include "benchmarks/common.h"
#include "competitors/hybrid_pgm_lipp.h"

template <typename Searcher>
void benchmark_64_hybrid_pgm_lipp(tli::Benchmark<uint64_t>& benchmark,
                                  bool pareto, const std::vector<int>& params) {
  (void)params;

  if (!pareto) {
    util::fail("Hybrid PGM LIPP's hyperparameter cannot be set");
  }

  benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 16, 10>>();
  benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 32, 10>>();
  benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 64, 10>>();
  benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 128, 10>>();
  benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 256, 10>>();
  benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 512, 10>>();
  benchmark.template Run<HybridPGMLIPP<uint64_t, Searcher, 1024, 10>>();
}

template <int record>
void benchmark_64_hybrid_pgm_lipp(tli::Benchmark<uint64_t>& benchmark, const std::string& filename) {
  if (filename.find("books_100M") != std::string::npos) {
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 32, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 32, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 256, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 256, 1>>();
  }
  if (filename.find("fb_100M") != std::string::npos) {
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 16, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 8, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 8, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 32, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 32, 1>>();
  }
  if (filename.find("osmc_100M") != std::string::npos) {
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 128, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 64, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 64, 1>>();
  }
  if (filename.find("wiki_100M") != std::string::npos) {
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 32, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, LinearSearch<record>, 32, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 8, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, ExponentialSearch<record>, 8, 1>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 10>>();
    benchmark.template Run<HybridPGMLIPP<uint64_t, BranchingBinarySearch<record>, 64, 1>>();
  }
}

INSTANTIATE_TEMPLATES_MULTITHREAD(benchmark_64_hybrid_pgm_lipp, uint64_t);