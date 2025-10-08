#include <benchmark/benchmark.h>
#include <algorithm>
#include <any>
#include "../Any.h"


template<typename AnyType, typename AnyValueType, typename ... Args>
static auto runAny(Args&& ... args) {
    AnyType a{std::in_place_type<AnyValueType>, std::forward<Args>(args)...};
    benchmark::ClobberMemory();
    AnyType b = std::move(a);
    benchmark::ClobberMemory();
    return any_cast<AnyValueType>(b);
}

template<typename Any>
static void benchWithInt(benchmark::State& state) {
  for (auto _ : state) 
    benchmark::DoNotOptimize(runAny<Any, int>(42));
}


BENCHMARK(benchWithInt<AnyOnePtr<8>>)->MinTime(5);
BENCHMARK(benchWithInt<std::any>)->MinTime(5);

BENCHMARK_MAIN();

