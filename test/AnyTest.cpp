#include "../Any.h"

#define BOOST_TEST_MODULE AnyTest

#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

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

using Storage = detail::StaticStorage<25>;

BOOST_AUTO_TEST_CASE(canInstantiate) {
    {
        Storage storage(S{kInt});
        BOOST_CHECK(storage.get<S>().i == kInt);
        Storage otherStorage = std::move(storage);
        BOOST_CHECK(otherStorage.get<S>().i == kInt);
        BOOST_CHECK(S::cnt == 2);
    }

    {
        Storage storage(S{kInt});
        {
            Storage otherStorage = std::move(storage);
            BOOST_CHECK(otherStorage.get<S>().i == kInt);
        }
        BOOST_CHECK(S::cnt == 1);
    }

    BOOST_CHECK(S::cnt == 0);
}

BOOST_AUTO_TEST_CASE(canMove) {
    {
        Storage storage(S{kInt});
        {
            Storage otherStorage = std::move(storage);
            BOOST_CHECK(otherStorage.get<S>().i == kInt);
        }
        BOOST_CHECK(S::cnt == 1);
    }

    BOOST_CHECK(S::cnt == 0);
}

BOOST_AUTO_TEST_CASE(canMoveAssign) {
    {
        Storage storage(S{kInt});
        {
            Storage otherStorage(S{42});
            otherStorage = std::move(storage);
            BOOST_CHECK(otherStorage.get<S>().i == kInt);
        }
        BOOST_CHECK(S::cnt == 1);
    }

    BOOST_CHECK(S::cnt == 0);
}
