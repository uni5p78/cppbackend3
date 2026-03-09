#include <iostream>

#include "urlencode.h"
using namespace std::literals;

int main() {
    std::string s;
    std::getline(std::cin, s);
    std::cout << UrlEncode(s) << std::endl;
}




        // char ch130 = 130;
        // uint8_t byte130 = ch130;
        // uint8_t res = byte130/16;
        // s = ch130;
    
    


