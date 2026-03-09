// main.cpp
#include <iostream>
#include "sql_server.h"
#include "cmd_manager.h"
#include "request_handler.h"
// #include <pqxx/pqxx>
// using pqxx::operator"" _zv;

using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: db_example <conn-string>\n"sv;
            return EXIT_SUCCESS;
        } else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }
        std::string param{argv[1]};
        // std::string param{"postgres://postgres:123@localhost:5432/anton_tast"};

        data::SqlServer sql{param};
        sql.InitDataDase();
        handler::RequestHandler hendler{sql};
        cmd::CmdManager(std::cin, std::cout, hendler).Run();

    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}