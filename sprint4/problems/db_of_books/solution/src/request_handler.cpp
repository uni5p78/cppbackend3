#include "request_handler.h"


namespace handler{


    enum class TypeOfRequest {
          AddBook
        , GetAllBooks
        , Exit
    };

struct ActionParam
{
    static inline const std::string ADD_BOOK = "add_book"s;
    static inline const std::string GET_ALL_BOOKS = "all_books"s;
    static inline const std::string EXIT = "exit"s;
};

struct RequestParam
{
    static inline const std::string ACTION = "action"s;
    static inline const std::string PAYLOAD = "payload"s;
};

struct PayloadParam
{
    static inline const std::string TITLE = "title"s;
    static inline const std::string AUTHOR = "author"s;
    static inline const std::string YEAR = "year"s;
    static inline const std::string ISBN = "ISBN"s;
};


    RequestHandler::RequestHandler(data::SqlServer& sql_server)
        :sql_server_(sql_server) {
    }

    TypeOfRequest GetTypeOfRequest(const json::JsonValue& request){
        auto action = request.GetParamAsString(RequestParam::ACTION);
        if (action == ActionParam::ADD_BOOK) 
            return TypeOfRequest::AddBook;
        if (action == ActionParam::GET_ALL_BOOKS) 
            return TypeOfRequest::GetAllBooks;
        return TypeOfRequest::Exit;
    }

    std::string RequestHandler::AddBook(const json::JsonValue& request) const {
        auto payload = request.GetParamAsObj(RequestParam::PAYLOAD);
        data::Book book;
        book.author = payload.GetParamAsString(PayloadParam::AUTHOR);
        book.title = payload.GetParamAsString(PayloadParam::TITLE);
        book.year = payload.GetParamAsInt(PayloadParam::YEAR);
        if (payload.IsNull(PayloadParam::ISBN)) {
            book.ISBN = std::nullopt;
        } else {
            book.ISBN = {payload.GetParamAsString(PayloadParam::ISBN)};
        }
        return json::GetResult(sql_server_.AddBook(book));
    }
    std::string RequestHandler::GetAllBooks(const json::JsonValue& request) const {
        return json::GetResult(sql_server_.GetAllBooks());;
        // return {};
    }
    std::string RequestHandler::Exit(const json::JsonValue& request) const {
        return {};
    }

    std::string RequestHandler::Handle(std::string&& command) const {
        auto request = json::ParseStr(command);
        auto type_request = GetTypeOfRequest(request);
        switch (type_request) {
            case TypeOfRequest::AddBook: 
                return AddBook(request);
            case TypeOfRequest::GetAllBooks: 
                return GetAllBooks(request);
            case TypeOfRequest::Exit: 
                return Exit(request);
        }
        return Exit(request);
    }

} // hendler cmd