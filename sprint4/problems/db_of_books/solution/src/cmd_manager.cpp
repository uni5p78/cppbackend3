#include "cmd_manager.h"


namespace cmd{

    CmdManager::CmdManager(std::istream& input, std::ostream& output, handler::RequestHandler& hendler)
        : input_{input}
        , output_{output}
        , handler_(hendler){

    }

    std::string CmdManager::HandlRequest(std::string&& command){
        return handler_.Handle(std::move(command));
    }

    void CmdManager::Run() {
        std::string line;
        while (std::getline(input_, line)) {
            // output_ << 
            auto response = HandlRequest(std::move(line));
            if (response.empty()) {
                break;
            };
            output_ << response << std::endl;
        }

    }

} // namespace cmd