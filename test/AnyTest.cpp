#include "../Any.h"

#define BOOST_TEST_MODULE AnyTest

#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mp11/algorithm.hpp>

namespace mp11 = boost::mp11;

template <typename AlignAs>
struct S {
    alignas(AlignAs) std::array<char, 15> payload;
    int i;
    inline static int cnt = 0;
    S(int i): i(i) {  cnt++; }
    S(S&& other): i(other.i) { cnt++; }
    S& operator=(S&& other) { i = other.i; return *this; }
    S(const S&) = delete;
    S& operator=(const S&) = delete;
    ~S() {
        cnt--;
    }
};

template <typename AlignAs>
struct C {
    alignas(AlignAs) std::array<char, 15> payload;
    int i;
    inline static int cnt = 0;
    C(int i): i(i) {  cnt++; }
    C(C&& other): i(other.i) { cnt++; }
    C& operator=(C&& other) { i = other.i;  return *this; }
    C(const C& other) { i = other.i; cnt++; }
    C& operator=(const C& other )  { i = other.i;  return *this; }
    ~C() {
        cnt--;
    }
};

inline constexpr int kInt = 13;

using StorageTypes = mp11::mp_list<
    AnyOnePtr<8, true>,
    AnyOnePtr<80, true>,
    AnyTwoPtrs<8, true>,
    AnyTwoPtrs<80, true>,
    AnyOnePtr<8, false>,
    AnyOnePtr<80, false>,
    AnyTwoPtrs<8, false>,
    AnyTwoPtrs<80, false>,
    detail::DynamicStorage>;

static_assert(alignof(__int128) > alignof(void*)); //make sure int128 has big alignment
static_assert(alignof(std::int32_t) < alignof(void*)); //make sure int32 has small alignment
using ValueTypes = mp11::mp_list<S<std::int32_t>, S<__int128>, C<std::int32_t>, C<__int128>>;

template <typename Storage_, typename Value_>
struct TestCase {
    using Storage = Storage_;
    using Value = Value_;
};

using TestCases = mp11::mp_product<TestCase, StorageTypes, ValueTypes>;

BOOST_AUTO_TEST_CASE_TEMPLATE(canInstantiateAndMove, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(std::in_place_type<Value>, kInt);
        BOOST_CHECK(any_cast<Value&>(storage).i == kInt);
        Storage otherStorage = std::move(storage);
        BOOST_CHECK(any_cast<Value&>(otherStorage).i == kInt);
    }
    BOOST_CHECK(T::Value::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canInstantiateAndMoveDifferentScope, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(Value{kInt});
        {
            Storage otherStorage = std::move(storage);
            BOOST_CHECK(any_cast<Value&>(otherStorage).i == kInt);
        }
    }

    BOOST_CHECK(Value::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canMove, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(Value{kInt});
        {
            Storage otherStorage = std::move(storage);
            BOOST_CHECK(any_cast<Value&>(otherStorage).i == kInt);
        }
    }

    BOOST_CHECK(Value::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canMoveAssign, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(Value{kInt});
        {
            Storage otherStorage(Value{42});
            otherStorage = std::move(storage);
            BOOST_CHECK(any_cast<Value&>(otherStorage).i == kInt);
        }
    }

    BOOST_CHECK(Value::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canSwap, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(Value{kInt});
        {
            Storage otherStorage(Value{42});
            std::swap(storage, otherStorage);
            BOOST_CHECK(any_cast<Value&>(otherStorage).i == kInt);
            BOOST_CHECK(any_cast<Value&>(storage).i == 42);
        }
    }

    BOOST_CHECK(Value::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canSwapHeterogeneously, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(Value{kInt});
        {
            Storage otherStorage(int{42});
            std::swap(storage, otherStorage);
            BOOST_CHECK(any_cast<Value&>(otherStorage).i == kInt);
            BOOST_CHECK(any_cast<int>(storage) == 42);
        }
    }

    BOOST_CHECK(Value::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canGetByValue, Storage, StorageTypes) {
    using Value = int;
    Storage storage(Value{kInt});
    auto value = any_cast<Value>(storage);
    BOOST_CHECK(value == kInt);
    value++;
    BOOST_CHECK(value == kInt + 1);
    BOOST_CHECK(any_cast<Value>(storage) == kInt);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canGetByRef, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(Value{kInt});
        auto& valueRef = any_cast<Value&>(storage);
        BOOST_CHECK(valueRef.i == kInt);
        valueRef.i++;
        BOOST_CHECK(valueRef.i == kInt + 1);
        BOOST_CHECK(any_cast<Value&>(storage).i == kInt + 1);
    }
    BOOST_CHECK(Value::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canGetByRefRef, T, TestCases) {
    using Storage = T::Storage;
    using Value = T::Value;
    {
        Storage storage(Value{kInt});
        auto value = any_cast<Value&&>(storage);
        BOOST_CHECK(value.i == kInt);
    }

    BOOST_CHECK(Value::cnt == 0);
}
