#include "htmldecode.h"
#include <vector>
#include <optional>
using namespace std::literals;

namespace{

struct HtmlMnemonics {
    std::string name;
    std::string name_shift;
    int size;
    char chr;

};

const std::vector<HtmlMnemonics> html_mnemonics {
      {"lt"s, "LT"s, 2, '<'} 
    , {"gt"s, "GT"s, 2, '>'}
    , {"amp"s, "AMP"s, 3, '&'}
    , {"apos"s, "APOS"s, 4, '\''}
    , {"quot"s, "QUOT"s, 4, '\"'}
};

}

const std::optional<const HtmlMnemonics*> GetHtmlMnemonic (std::string_view rest){
    for (const HtmlMnemonics & html_chr : html_mnemonics){
        if (html_chr.size > rest.size()){
            continue;
        }
        auto rest2 = rest.substr(0, html_chr.size);
        if((html_chr.name == rest2 || html_chr.name_shift == rest2)){
            return &html_chr;
        }
    }
    return std::nullopt;
}


std::string HtmlDecode(std::string_view str) {
    if(str.empty()) {
        return {};
    }
    std::string res;
    res.reserve(str.size());
    char amp = '&';
    for (auto pos = str.find(amp, 0); pos != std::string::npos; pos = str.find(amp, 0)){
        // size_t chrs_ostatok = str.size()-pos-1;
        res.append(str.substr(0, pos));
        if (const auto html_chr = GetHtmlMnemonic(str.substr(pos+1))){
            res.push_back((*html_chr)->chr);
            str.remove_prefix(pos + 1 + (*html_chr)->size);
            if (!str.empty() && str[0] == ';'){
                str.remove_prefix(1);
            }
        } else {
            res.push_back(amp);
            str.remove_prefix(pos + 1);
        }
        

    }
    res.append(str);
    return res;
}
