#include "sql_server.h"

using namespace std::literals;

namespace data{

    SqlServer::SqlServer(std::string options) 
    :conn_(options)
    {}

    void SqlServer::InitDataDase(){
        // Создаём транзакцию. Это понятие будет разобрано в следующих уроках.
        // Транзакция нужна, чтобы выполнять запросы.
        pqxx::work w(conn_);

        // Используя транзакцию создадим таблицу в выбранной базе данных:
        w.exec(
            "CREATE TABLE IF NOT EXISTS books ("
            "id SERIAL PRIMARY KEY"
            ", title varchar(100) NOT NULL"
            ", author varchar(100) NOT NULL"
            ", year integer NOT NULL"
            ", ISBN char(13) UNIQUE"
            ");"_zv);

        // Применяем все изменения
        w.commit();
    }


    bool SqlServer::AddBook(Book book) {
        pqxx::work w(conn_);
        w.exec(
            "INSERT INTO books (title, author, year, ISBN) "
            "VALUES ('" + w.esc(book.title) + "'"
            ", '" + w.esc(book.author) + "'"
            ", " + std::to_string(book.year) + ""
            ", " +  (book.ISBN ? w.quote(*(book.ISBN)) : "NULL") +
            ");");

        // Применяем все изменения
        w.commit();
        return true;
    }

    Books SqlServer::GetAllBooks() {
        Books res;
        pqxx::read_transaction r(conn_);
        auto query_text = "SELECT id, title, author, year, ISBN FROM books ORDER BY year desc, title, author, ISBN"_zv;

        // Выполняем запрос и итерируемся по строкам ответа
        for (auto [id, title, author, year, ISBN] : r.query<int, std::string_view, std::string_view, int, std::optional<std::string>>(query_text)) {
            res.emplace_back(id, std::string{title}, std::string{author}, year, ISBN);
        }

        return res;
    }

} // namespace data
