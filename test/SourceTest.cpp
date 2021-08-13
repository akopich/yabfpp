#include "../Source.h"

#include <numeric>

#define BOOST_TEST_MODULE SourceTest

#include <boost/test/included/unit_test.hpp>

using namespace boost::unit_test;


void testSingle(std::vector<std::string>& v) {
    auto skipUnderscore = [](const char c) { return c == '_'; };
    Source source(v, skipUnderscore);

    std::string expected = std::accumulate(v.begin(), v.end(), std::string(""));
    expected.erase(std::remove(expected.begin(), expected.end(), '_'), expected.end());

    std::string traversed(source.begin(), source.end());
    std::cout << traversed << std::endl;

    BOOST_CHECK(expected == traversed);
}

BOOST_AUTO_TEST_CASE(test) {
    std::vector<std::vector<std::string>> v = {
            {"____asd_______f__", "_z__xcv____", "et__r_y______"},
            {},
            {"_____",             "asdf",        "___"},
            {"___",               "___"},
            {"",                  "asdf"},
            {"",                  "asdf",        "", "wer", "", "", "", "mbvn"},
    };
    std::for_each(v.begin(), v.end(), testSingle);
}