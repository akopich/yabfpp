#include "../Any.h"

#define BOOST_TEST_MODULE AnyTest

#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

using namespace boost::unit_test;

struct S {
    std::array<char, 15> payload;
    inline static int cnt = 0;
    S() {  cnt++; }
    S(S&&) { cnt++; }
    S& operator=(S&&) { cnt++; return *this; }
    S(S&) = delete;
    S& operator=(S&) = delete;
    ~S() {
        cnt--;
    }
};

BOOST_AUTO_TEST_CASE(canInstantiate) {
    {
        using Storage = detail::StaticStorage<25>;
        Storage storage(S{});
        Storage otherStorage = std::move(storage);
        BOOST_CHECK(S::cnt == 2);
    }

    {
        using Storage = detail::StaticStorage<25>;
        Storage storage(S{});
        {
            Storage otherStorage = std::move(storage);
        }
        BOOST_CHECK(S::cnt == 1);
    }

    BOOST_CHECK(S::cnt == 0);
}

BOOST_AUTO_TEST_CASE(canMove) {
    {
        using Storage = detail::StaticStorage<25>;
        Storage storage(S{});
        {
            Storage otherStorage = std::move(storage);
        }
        BOOST_CHECK(S::cnt == 1);
    }

    BOOST_CHECK(S::cnt == 0);
}

BOOST_AUTO_TEST_CASE(canMoveAssign) {
    {
        using Storage = detail::StaticStorage<25>;
        Storage storage(S{});
        {
            Storage otherStorage(S{});
            otherStorage = std::move(storage);
        }
        BOOST_CHECK(S::cnt == 1);
    }

    BOOST_CHECK(S::cnt == 0);
}
