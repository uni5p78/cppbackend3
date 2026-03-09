#pragma once

#include <filesystem>
#include <string>
#include <boost/json.hpp>
#include "sql_server.h"

using std::literals::string_literals::operator""s;

namespace json 
{

    struct ResultFields
    {
        static inline const std::string RESULT = "result"s;
        static inline const std::string ID = "id"s;
        static inline const std::string TITLE = "title"s;
        static inline const std::string AUTHOR = "author"s;
        static inline const std::string YEAR = "year"s;
        static inline const std::string ISBN = "ISBN"s;

    };

struct PayloadParam
{
};
    std::string GetResult(bool res_query);
    std::string GetResult(data::Books);

    class JsonValue;
    using ArrayJsonValue = std::vector<JsonValue>;

    class JsonValue {
    public:
        JsonValue(const boost::json::value& value);
        const ArrayJsonValue GetParamAsArray(const std::string& name_array) const;
        std::string GetParamAsString(const std::string& name_param) const;
        int GetParamAsInt(const std::string& name_param) const;
        double GetParamAsDouble(const std::string& name_param) const; 
        JsonValue GetParamAsObj(const std::string& name_param) const; 
        int GetSizeParamArray(const std::string& name_array) const;
        bool ContainsParam(const std::string& name_param) const;
        bool IsNull(const std::string& name_param) const;
        void SetParentPtr(std::shared_ptr<boost::json::value> ptr);
        boost::json::value GetValue();

    private:
        const boost::json::value& value_;
        std::shared_ptr<boost::json::value> parent_ptr_;
    };

    JsonValue ParseFile(const std::filesystem::path& json_path);
    JsonValue ParseStr(const std::string& str_json);  
} // namespace boost_json 

