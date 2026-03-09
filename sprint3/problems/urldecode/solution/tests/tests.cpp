#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);
    BOOST_TEST(UrlDecode("Backend"sv) == "Backend"s);
    BOOST_TEST(UrlDecode("%48%65%6C%6C%6F%21"sv) == "Hello!"s);
    BOOST_CHECK_THROW(UrlDecode("%48%6M%6C%6C%6F%21"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("%48%6%6C%6C%6F%21"sv), std::invalid_argument);
    BOOST_TEST(UrlDecode("Hello+World"sv) == "Hello World"s);
    // Напишите остальные тесты для функции UrlDecode самостоятельно
}