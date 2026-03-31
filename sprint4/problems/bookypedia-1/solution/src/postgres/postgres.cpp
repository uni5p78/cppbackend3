#include "postgres.h"

#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    if(author.GetName().empty()){
        throw std::logic_error("");
    }
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

info::Authors AuthorRepositoryImpl::GetAuthors() const {
    info::Authors res;
    pqxx::read_transaction r(connection_);
    auto query_text = "SELECT id, name FROM authors ORDER BY name"_zv;
    // Выполняем запрос и итерируемся по строкам ответа
    for (auto [id, name] : r.query<std::string_view, std::string_view>(query_text)) {
        res.emplace_back(std::string(id), std::string(name));
    }
    return res;
}

void BookRepositoryImpl::Save(const domain::Book& Book) {
    if(Book.GetAuthorId().ToString().empty() || Book.GetTitle().empty() || Book.GetPublicationYear() <= 0){
        throw std::logic_error("");
    }
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO Books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4)
ON CONFLICT (id) DO UPDATE SET author_id=$2, title=$3, publication_year=$4;
)"_zv,
        Book.GetId().ToString(), Book.GetAuthorId().ToString()
        , Book.GetTitle(), Book.GetPublicationYear());
    work.commit();
}

info::Books BookRepositoryImpl::GetBooks() const {
    info::Books res;
    pqxx::read_transaction r(connection_);
    auto query_text = "SELECT title, publication_year FROM books ORDER BY title"_zv;
    // Выполняем запрос и итерируемся по строкам ответа
    for (auto [title, year] : r.query<std::string_view, int>(query_text)) {
        res.emplace_back(std::string(title), year);
    }
    return res;
}

info::Books BookRepositoryImpl::GetAuthorBooks(const std::string& author_id) const {
    info::Books res;
    pqxx::read_transaction r(connection_);
    auto query_text = "SELECT title, publication_year FROM books WHERE author_id = '" 
    + author_id + "' ORDER BY publication_year, title";
    // Выполняем запрос и итерируемся по строкам ответа
    for (auto [title, year] : r.query<std::string_view, int>(query_text)) {
        res.emplace_back(std::string(title), year);
    }
    return res;
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL 
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
    author_id UUID NOT NULL ,
    title varchar(100) NOT NULL ,
    publication_year integer 
); 
)"_zv);
    // коммитим изменения
    work.commit();
}

}  // namespace postgres

/*
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL CHECK (name <> '')
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
    author_id UUID NOT NULL ,
    title varchar(100) NOT NULL CHECK (title <> ''),
    publication_year integer  CHECK (publication_year > 1000)
); 
)"_zv);
 */