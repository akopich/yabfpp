#include <benchmark/benchmark.h>
#include <algorithm>
#include <any>
#include <utility> 
#include "../Any.h"


template<typename AnyType, typename AnyValueType, typename ... Args>
static auto runAny(Args&& ... args) {
    AnyType a{std::in_place_type<AnyValueType>, std::forward<Args>(args)...};
    benchmark::ClobberMemory();
    AnyType b(std::move(a));
    benchmark::ClobberMemory();
    return any_cast<AnyValueType>(b);
}

template<typename Any>
static void benchWithInt(benchmark::State& state) {
  for (auto _ : state) {
    runAny<Any, int>(42);
    benchmark::ClobberMemory(); 
  }
}

template<typename Any>
static void benchCtorInt(benchmark::State& state) {
  for (auto _ : state) 
    benchmark::DoNotOptimize(Any{42});
}

template<typename Any>
static void benchGetInt(benchmark::State& state) {
  Any any{42};
  benchmark::ClobberMemory(); 
  for (auto _ : state) 
    benchmark::DoNotOptimize(any_cast<int>(any));
}

static void benchGetWithGet(benchmark::State& state) {
  using Any = AnyOnePtr<8>;
  Any any{42};
  for (auto _ : state) 
    benchmark::DoNotOptimize(any.template get<int>());
}

template<typename Any>
static void benchMoveInt(benchmark::State& state) {
  for (auto _ : state)  {
    Any source{42};      
    benchmark::ClobberMemory(); 
    benchmark::DoNotOptimize(Any{std::move(source)});
  }
}

template<typename Any>
static void benchSwapInt(benchmark::State& state) {
  Any a{42};      
  Any b{37};      
  for (auto _ : state)  {
    using std::swap;
    swap(a,b);
    benchmark::ClobberMemory(); 
  }
}

constexpr static int kMinTime = 1;

BENCHMARK(benchWithInt<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchWithInt<std::any>)->MinTime(kMinTime);
BENCHMARK(benchCtorInt<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchCtorInt<std::any>)->MinTime(kMinTime);
BENCHMARK(benchGetInt<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchGetWithGet)->MinTime(kMinTime);
BENCHMARK(benchGetInt<std::any>)->MinTime(kMinTime);
BENCHMARK(benchMoveInt<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchMoveInt<std::any>)->MinTime(kMinTime);
BENCHMARK(benchSwapInt<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchSwapInt<std::any>)->MinTime(kMinTime);

BENCHMARK_MAIN();

