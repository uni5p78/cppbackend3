#pragma once

#include <boost/program_options.hpp>
#include <optional>
#include <chrono>


namespace comand_line 
{
    struct Args {
        int tick_period{};
        int save_state_period{};
        std::string config_file{};
        std::string www_root{};
        std::string state_file{};
        bool randomize_spawn_points{};
    }; 

    [[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) ;
} 

