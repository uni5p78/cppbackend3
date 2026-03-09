#include "urldecode.h"

#include <charconv>
#include <stdexcept>
#include <string_view>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cmath>

using namespace std::literals;

char HexchToInt(char hex_ch) {
    if(hex_ch >= '0' && hex_ch <= '9'){
        return hex_ch - '0';
    } 
    if(hex_ch >= 'A' && hex_ch <= 'F'){
        return hex_ch - 'A' + 10;
    }
    if(hex_ch >= 'a' && hex_ch <= 'f'){
        return hex_ch - 'a' + 10;
    }
    throw std::invalid_argument("Invalid HEX "+hex_ch); // если не смогли перевести число из Hex

}

char fromHex(std::string_view hexValue){
    size_t ch_count =  hexValue.size();
    char res = 0;
    for(const char hex_ch : hexValue) {
        ch_count--;
        res += HexchToInt(hex_ch) * std::pow(16, ch_count);
    };
    return res;
}

std::string UrlDecode(std::string_view str) {
    if(str.empty()) {
        return {};
    }
    std::string res;
    res.reserve(str.size());
    char prc = '%';
    for (auto pos = str.find(prc, 0); pos != std::string::npos; pos = str.find(prc, 0)) {
        if (pos+3>str.size() ) {
            throw std::invalid_argument("Invalid HEX"); // если не смогли перевести число из Hex
        }
        res.append(str.substr(0, pos));
        res.push_back(fromHex(str.substr(pos+1, 2)));
        str.remove_prefix(pos + 3);
    }
    res.append(str);
    boost::replace_all(res, "+"s, " "s);
    return res;
}
