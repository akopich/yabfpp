#include <benchmark/benchmark.h>
#include <algorithm>
#include <any>
#include <limits>
#include <random>
#include <ranges>
#include <utility> 
#include <limits> 
#include "../Any.h"


template<typename AnyType, typename AnyValueType, typename ... Args>
static auto runAny(Args&& ... args) {
    AnyType a{std::in_place_type<AnyValueType>, std::forward<Args>(args)...};
    benchmark::ClobberMemory();
    AnyType b(std::move(a));
    benchmark::ClobberMemory();
    return any_cast<AnyValueType>(b);
}

template<typename Any, typename ValueType, auto Value>
static void benchWithValue(benchmark::State& state) {
  for (auto _ : state) {
    runAny<Any, ValueType>(Value);
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


template <size_t N, typename Any, typename ValueType>
static void benchVectorConstruction(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> intDist(1, std::numeric_limits<int>::max());
    std::vector<ValueType> ints(N);
    std::ranges::generate(ints, [&] {return intDist(gen);});

    for (auto _ : state) {
        auto anys = ints | std::ranges::views::transform([&](auto i) { 
                                 return Any{std::in_place_type<ValueType>, i};
                            }) 
                         | std::ranges::to<std::vector>();
        benchmark::ClobberMemory(); 
    }
}

constexpr static int kMinTime = 1;

template <typename Any>
static auto benchWithInt = benchWithValue<Any, int, 42>;

BENCHMARK(benchWithInt<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchWithInt<std::any>)->MinTime(kMinTime);

template <typename Any>
static auto benchWithInt64 = benchWithValue<Any, std::uint64_t, 0xDEADBEEF>;

BENCHMARK(benchWithInt64<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchWithInt64<std::any>)->MinTime(kMinTime);

template <typename Any>
static auto benchWithInt128 = benchWithValue<Any, __int128, 0xDEADDEADBEEF>;

BENCHMARK(benchWithInt128<AnyOnePtr<8>>)->MinTime(kMinTime);
BENCHMARK(benchWithInt128<std::any>)->MinTime(kMinTime);

template <size_t N, typename Any>
static auto benchVectorConstructionInt = benchVectorConstruction<N, Any, int>;

template <size_t N, typename Any>
static auto benchVectorConstructionInt64 = benchVectorConstruction<N, Any, std::uint64_t>;

template <size_t N, typename Any>
static auto benchVectorConstructionInt128 = benchVectorConstruction<N, Any, __int128_t>;

BENCHMARK(benchVectorConstructionInt<1, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt<1, std::any>);

BENCHMARK(benchVectorConstructionInt<10, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt<10, std::any>);

BENCHMARK(benchVectorConstructionInt<100, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt<100, std::any>);

BENCHMARK(benchVectorConstructionInt<1000, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt<1000, std::any>);

BENCHMARK(benchVectorConstructionInt<100000, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt<100000, std::any>);

BENCHMARK(benchVectorConstructionInt128<1, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt128<1, std::any>);

BENCHMARK(benchVectorConstructionInt128<10, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt128<10, std::any>);

BENCHMARK(benchVectorConstructionInt128<100, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt128<100, std::any>);

BENCHMARK(benchVectorConstructionInt128<1000, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt128<1000, std::any>);

BENCHMARK(benchVectorConstructionInt128<100000, AnyOnePtr<8>>);
BENCHMARK(benchVectorConstructionInt128<100000, std::any>);

//BENCHMARK(benchCtorInt<AnyOnePtr<8>>)->MinTime(kMinTime);
//BENCHMARK(benchCtorInt<std::any>)->MinTime(kMinTime);
//BENCHMARK(benchGetInt<AnyOnePtr<8>>)->MinTime(kMinTime);
//BENCHMARK(benchGetWithGet)->MinTime(kMinTime);
//BENCHMARK(benchGetInt<std::any>)->MinTime(kMinTime);
//BENCHMARK(benchMoveInt<AnyOnePtr<8>>)->MinTime(kMinTime);
//BENCHMARK(benchMoveInt<std::any>)->MinTime(kMinTime);
//BENCHMARK(benchSwapInt<AnyOnePtr<8>>)->MinTime(kMinTime);
//BENCHMARK(benchSwapInt<std::any>)->MinTime(kMinTime);

BENCHMARK_MAIN();

