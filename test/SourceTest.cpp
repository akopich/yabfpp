#include "../Source.h"

#include <algorithm>
#include <iostream>

#define BOOST_TEST_MODULE SourceTest

#include <boost/test/included/unit_test.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>

using namespace boost::unit_test;
namespace bdata = boost::unit_test::data;

const auto* testCases = new std::vector<std::vector<std::string>>{
    {"____asd_______f__", "_z__xcv____", "et__r_y______"},
    {},
    {"_____",             "asdf",        "___"},
    {"___",               "___"},
    {"",                  "asdf"},
    {"",                  "asdf",        "", "wer", "", "", "", "mbvn"},
};

namespace std {
ostream& operator<<(ostream& os, const vector<string>& vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i < vec.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}
}

BOOST_DATA_TEST_CASE(test, bdata::make(*testCases)) {
    auto skipUnderscore = [](const char c) { return c == '_'; };
    Source source(sample, skipUnderscore);

    std::string expected = std::ranges::fold_left(sample, std::string{""}, std::plus{});
    std::erase(expected, '_');

    std::string traversed(source.begin(), source.end());
    std::cout << traversed << std::endl;

    BOOST_CHECK(expected == traversed);
}
