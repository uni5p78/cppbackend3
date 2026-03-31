#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}


void UseCasesImpl::AddBook(const std::string& autor_id
    , const std::string& title, const int publication_year) {
    books_.Save(Book(BookId::New(), AuthorId::FromString(autor_id), title, publication_year));
}

info::Authors UseCasesImpl::GetAuthors() const {
    return authors_.GetAuthors();
}

info::Books UseCasesImpl::GetBooks() const {
    return books_.GetBooks();
}

info::Books UseCasesImpl::GetAuthorBooks(const std::string& author_id) const {
    return books_.GetAuthorBooks(author_id);
}

}  // namespace app
