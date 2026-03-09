#pragma once
#include <pqxx/pqxx>
#include <optional>

using pqxx::operator"" _zv;

namespace data{
struct Book {
    size_t id ;
    std::string title ;
    std::string author ;
    int year;
    std::optional<std::string> ISBN ;
};

using Books = std::vector<Book>;

class SqlServer {
public: 
    SqlServer(std::string);
    void InitDataDase();
    bool AddBook(Book book);
    Books GetAllBooks();
private:
    pqxx::connection conn_;
};


} // namespace data