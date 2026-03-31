#pragma once
// #include <iosfwd>
// #include <optional>
#include <string>
#include <vector>
#include <iostream>

namespace info {

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};


struct BookInfo {
    std::string title;
    int publication_year;
};
using Authors = std::vector<AuthorInfo>;
using Books = std::vector<BookInfo>;

// }  // namespace detail
// namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author);

std::ostream& operator<<(std::ostream& out, const BookInfo& book);

}  // namespace detail
