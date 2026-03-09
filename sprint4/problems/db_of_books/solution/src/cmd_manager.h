#pragma once
#include <iostream>
#include "request_handler.h"

namespace cmd{

class CmdManager{
public:
    CmdManager(std::istream& input, std::ostream& output, handler::RequestHandler& hendler);
    void Run();
private:
    std::istream& input_;
    std::ostream& output_;
    handler::RequestHandler& handler_;

    std::string HandlRequest(std::string&& command);
};

}