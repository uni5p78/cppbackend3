#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode(""sv), ""s);
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
    EXPECT_EQ(UrlEncode("Hello!"sv), "Hello%21"s);
    EXPECT_EQ(UrlEncode("Hello World "sv), "Hello+World+"s);
    char ch30 = 30;
    std::string str = "Hello"s + ch30;
    EXPECT_EQ(UrlEncode(str), "Hello%1E"s);
    char ch130 = 130;
    str = "Hello"s + ch130;
    EXPECT_EQ(UrlEncode(str), "Hello%82"s);
}

/* Напишите остальные тесты самостоятельно */
