#pragma once
#include "json.h"
#include "sql_server.h"

namespace handler{

class RequestHandler{
public:
    RequestHandler(data::SqlServer& sql_server);
    std::string Handle(std::string&& command) const;

    // void Run();
private:
    data::SqlServer& sql_server_;
    std::string AddBook(const json::JsonValue& request) const;
    std::string GetAllBooks(const json::JsonValue& request) const;
    std::string Exit(const json::JsonValue& request) const;
};

} // namespace hendler