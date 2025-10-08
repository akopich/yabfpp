#include "BFProgramGenerator.h"
#include "../parser.h"
#include <benchmark/benchmark.h>
#include <algorithm>

static auto program = generateBFProgram(100000, /*seed=*/ 42);

static void runParser(benchmark::State& state) {
  for (auto _ : state) 
    Parser{}.parse(program);
}

BENCHMARK(runParser)->Repetitions(10)
    ->ComputeStatistics("min", [](const std::vector<double>& v) {
            return *std::ranges::min_element(v);
            });

BENCHMARK_MAIN();

