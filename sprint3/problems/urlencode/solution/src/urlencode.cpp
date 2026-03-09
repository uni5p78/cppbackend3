// #include <charconv>
#include "urlencode.h"
#include <sstream>
#include <string>
// #include <format>

using namespace std::literals;

std::string CharEncode(const char chr){
    std::string res;
    res.reserve(3);
    const std::string spec_symbols = "!#$&'()*+,/:;=?@[]"s;
    if(chr == ' '){
        res = "+"s;
    } else if(auto pos = spec_symbols.find(chr, 0); pos != std::string::npos 
    || chr < 32 || chr > 128){
        uint8_t byte = chr;
        // res = "%"s;

        std::stringstream ss;
        ss << '%' << std::hex << std::uppercase << byte/16 << byte%16;
        res = ss.str();
        // return ss.str();
        // return std::format("{:x}", chr);
    } else {
        res = {chr};
    }
    return res;
}

std::string UrlEncode(std::string_view str) {
    // Напишите реализацию самостоятельно
    std::string res;
    for(const char chr : str){
        res.append(CharEncode(chr));
    }
    return res;
}
