#pragma once
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors, domain::BookRepository& books)
        : authors_{authors} 
        , books_{books} 
        {
    }

    void AddAuthor(const std::string& name) override;
    void AddBook(const std::string& autor_id, const std::string& title, const int publication_year) override;
    info::Authors GetAuthors() const override;
    info::Books GetBooks() const override;
    info::Books GetAuthorBooks(const std::string& author_id) const override;

private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
};

}  // namespace app
