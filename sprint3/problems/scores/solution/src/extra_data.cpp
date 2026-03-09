#include "extra_data.h"


namespace extra_data{

    void ExtraData::SetLootTypesForMap(model::Map::Id map_id, boost::json::value value) {
        using namespace std::literals;
        auto [it, inserted] = map_id_to_jvalue_.emplace(map_id, value);

        if (!inserted) {
            throw std::invalid_argument("Loot types for Map with id "s + *map_id + " already exists"s);
        }
    }

    boost::json::value ExtraData::GetLootTypesForMap(model::Map::Id map_id) {
        return map_id_to_jvalue_.at(map_id);
    }



} // namespace extra_data
