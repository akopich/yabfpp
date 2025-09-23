#include "../Any.h"

#define BOOST_TEST_MODULE AnyTest

#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/mpl/list.hpp>

using namespace boost::unit_test;

struct S {
    std::array<char, 15> payload;
    int i;
    inline static int cnt = 0;
    S(int i): i(i) {  cnt++; }
    S(S&& other): i(other.i) { cnt++; }
    S& operator=(S&& other) { i = other.i; cnt++; return *this; }
    S(S&) = delete;
    S& operator=(S&) = delete;
    ~S() {
        cnt--;
    }
};

inline constexpr int kInt = 13;

//using Storage1 = detail::StaticStorage<detail::MemManagerOnePtr, detail::mkMemManagerOnePtr, 25>;
using Storage2 = detail::StaticStorage<detail::MemManagerTwoPtrs, detail::mkMemManagerTwoPtrs,  detail::mkMemManagerTwoPtrsDynamic,  8>;
using Storage2Big = detail::StaticStorage<detail::MemManagerTwoPtrs, detail::mkMemManagerTwoPtrs,  detail::mkMemManagerTwoPtrsDynamic,  80>;

using StorageTypes = boost::mpl::list<
    // Storage1, 
    Storage2, Storage2Big, detail::DynamicStorage>;

BOOST_AUTO_TEST_CASE_TEMPLATE(canInstantiate, Storage, StorageTypes) {
    {
        Storage storage(std::in_place_type<S>, kInt);
        BOOST_CHECK(any_cast<S>(storage).i == kInt);
        Storage otherStorage = std::move(storage);
        BOOST_CHECK(any_cast<S>(otherStorage).i == kInt);
    }

    {
        Storage storage(S{kInt});
        {
            Storage otherStorage = std::move(storage);
            BOOST_CHECK(any_cast<S>(otherStorage).i == kInt);
        }
    }

    BOOST_CHECK(S::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canMove, Storage, StorageTypes) {
    {
        Storage storage(S{kInt});
        {
            Storage otherStorage = std::move(storage);
            BOOST_CHECK(any_cast<S>(otherStorage).i == kInt);
        }
    }

    BOOST_CHECK(S::cnt == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(canMoveAssign, Storage, StorageTypes) {
    {
        Storage storage(S{kInt});
        {
            Storage otherStorage(S{42});
            otherStorage = std::move(storage);
            BOOST_CHECK(any_cast<S>(otherStorage).i == kInt);
        }
    }

    BOOST_CHECK(S::cnt == 0);
}
