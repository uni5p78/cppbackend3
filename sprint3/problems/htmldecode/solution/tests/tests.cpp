#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
    CHECK(HtmlDecode("&amp"sv) == "&"s);
    CHECK(HtmlDecode("&LT&GT"sv) == "<>"s);
    CHECK(HtmlDecode("&lt&AMP&gT"sv) == "<&&gT"s);
    CHECK(HtmlDecode("&lthel&quotlo&gt"sv) == "<hel\"lo>"s);
    CHECK(HtmlDecode("&lthel&amlo&gt &;"sv) == "<hel&amlo> &;"s);
    CHECK(HtmlDecode("&lt;hel&aMp;l&aposo&gt;"sv) == "<hel&aMp;l\'o>"s);
}

// Напишите недостающие тесты самостоятельно
