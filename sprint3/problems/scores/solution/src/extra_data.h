#pragma once
#include "model.h"
#include <boost/json.hpp>


namespace extra_data{


class ExtraData {
public:
    void SetLootTypesForMap(model::Map::Id map_id, boost::json::value value);
    boost::json::value GetLootTypesForMap(model::Map::Id map_id) const ;
private:
    using MapIdHasher = util::TaggedHasher<model::Map::Id>;
    using MapIdToJsonValue = std::unordered_map<model::Map::Id, boost::json::value, MapIdHasher>;
    MapIdToJsonValue map_id_to_jvalue_;
};


} // namespace extra_data
