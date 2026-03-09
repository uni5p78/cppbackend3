#pragma once

#include <filesystem>

#include "model.h"
#include "extra_data.h"

namespace json_loader 
{
    void LoadGame(model::Game& game,const std::filesystem::path& json_path
    , extra_data::ExtraData& data);
}  
