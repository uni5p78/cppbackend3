#pragma once
#include "../domain/author_fwd.h"
#include "../ui/struct_info.h"
#include <vector>

#include <string>

namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual void AddBook(const std::string& autor_id, const std::string& title, const int publication_year) = 0;
    virtual info::Authors GetAuthors() const = 0;
    virtual info::Books GetBooks() const = 0;
    virtual info::Books GetAuthorBooks(const std::string& author_id) const = 0;
protected:
    ~UseCases() = default;
};

}  // namespace app
