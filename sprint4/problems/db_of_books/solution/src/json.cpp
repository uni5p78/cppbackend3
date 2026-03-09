// Этот файл служит для подключения реализации библиотеки Boost.Json
#include "json.h"

#include <fstream>
#include <sstream>
#include <iostream>
  

using namespace std::literals;

namespace json {

std::string GetResult(bool res_query){
    boost::json::object obj;
    obj[ResultFields::RESULT] = res_query;
    return serialize(obj);
}

std::string GetResult(data::Books books){
    boost::json::array arr;
    for (const auto& book : books) {
        boost::json::object obj;
        obj[ResultFields::ID] = book.id;
        obj[ResultFields::TITLE] = book.title;
        obj[ResultFields::AUTHOR] = book.author;
        obj[ResultFields::YEAR] = book.year;
        if (book.ISBN) {
            obj[ResultFields::ISBN] = *(book.ISBN);
        } else {
            obj[ResultFields::ISBN].emplace_null();
        }
        arr.emplace_back(std::move(obj));
    }
    return serialize(arr);
}

std::string SerializeEmptyJsonObject(){
    boost::json::object obj;
    return serialize(obj);
}

JsonValue::JsonValue(const boost::json::value& value)
: value_(value){
}

const std::vector<JsonValue> JsonValue::GetParamAsArray(const std::string& name_array) const {
    auto& json_array = value_.as_object().at(name_array).as_array();
    // json_array.size();
    return {json_array.begin(), json_array.end()};
}

std::string JsonValue::GetParamAsString(const std::string& name_param) const {
    return value_.at(name_param).get_string().c_str();
}

int JsonValue::GetParamAsInt(const std::string& name_param) const {
    return static_cast<int>(value_.as_object().at(name_param).as_int64());
}

double JsonValue::GetParamAsDouble(const std::string& name_param) const {
    return static_cast<double>(value_.as_object().at(name_param).as_double());
}

int JsonValue::GetSizeParamArray(const std::string& name_array) const {
    return value_.as_object().at(name_array).as_array().size();
}

JsonValue JsonValue::GetParamAsObj(const std::string& name_param) const {
    return {value_.as_object().at(name_param)};
}

bool JsonValue::ContainsParam(const std::string& name_param) const{
    return value_.as_object().contains(name_param);
}

bool JsonValue::IsNull(const std::string& name_param) const{
     return value_.as_object().at(name_param).is_null();
}

void JsonValue::SetParentPtr(std::shared_ptr<boost::json::value> ptr){
    parent_ptr_ = ptr;
}

boost::json::value JsonValue::GetValue() {
    return value_;
}




JsonValue ParseFile(const std::filesystem::path& json_path){
    std::ifstream in_file(json_path);
    if (!in_file){
        throw std::runtime_error("Can't open config file: "s + json_path.c_str());
    };
    std::ostringstream sstr;
    sstr << in_file.rdbuf();
    return ParseStr(sstr.str());
}

JsonValue ParseStr(const std::string& str_json){
    try {
        //Создаем в куче дерево Json, чтобы потом не него могли ссылаться объекты JsonValue
        auto val_shptr = std::make_shared<boost::json::value>(boost::json::parse(str_json)); 
        JsonValue res{*val_shptr};
        // Сохраняем shared ссылку на дерево Json в первом объекте JsonValue, чтобы продлить 
        // срок жизни дерева до завершения обработки информации
        res.SetParentPtr(val_shptr);
        return res;
    }
    catch (std::exception& ex) {
        throw std::runtime_error("ParseStr(): Error parse string in json object -> "s + ex.what());
    }
}



} // namespace boost_json 

