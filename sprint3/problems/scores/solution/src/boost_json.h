#pragma once

#include <filesystem>
#include <string>
#include <boost/json.hpp>
#include "application.h"
#include "extra_data.h"   

using std::literals::string_literals::operator""s;

struct ErrorMesFields
{
    static inline const std::string CODE = "code"s;
    static inline const std::string MESSAGE = "message"s;
};

struct RoadCoord
{
    static inline const std::string START_X = "x0"s;
    static inline const std::string START_Y = "y0"s;
    static inline const std::string END_X = "x1"s;
    static inline const std::string END_Y = "y1"s;
};

struct JoinRequestFields
{
    static inline const std::string USER_NAME = "userName"s;
    static inline const std::string MAP_ID = "mapId"s;
};

struct MapFields
{
    static inline const std::string ID = "id"s;
    static inline const std::string NAME = "name"s;
    static inline const std::string ROADS = "roads"s;
    static inline const std::string BILDINGS = "buildings"s;
    static inline const std::string OFFICES = "offices"s;
    static inline const std::string DOG_SPEED = "dogSpeed"s;
    static inline const std::string LOOT_TYPES = "lootTypes"s;
    static inline const std::string LOOT_VALUE = "value"s;
    static inline const std::string BAG_CAPACITY = "bagCapacity"s;
};

struct OfficeFields
{
    static inline const std::string ID = "id"s;
    static inline const std::string POSITION_X = "x"s;
    static inline const std::string POSITION_Y = "y"s;
    static inline const std::string OFFSET_X = "offsetX"s;
    static inline const std::string OFFSET_Y = "offsetY"s;
};

struct BuildingFields
{
    static inline const std::string POSITION_X = "x"s;
    static inline const std::string POSITION_Y = "y"s;
    static inline const std::string WIDTH = "w"s;
    static inline const std::string HEIGHT = "h"s;
};

struct PlayerFields
{
    static inline const std::string TOKEN = "authToken"s;
    static inline const std::string PAYER_ID = "playerId"s;
};

struct DogFields
{
    static inline const std::string NAME = "name"s;
};

struct GameSateFields
{
    static inline const std::string POS = "pos"s;
    static inline const std::string TYPE = "type"s;
    static inline const std::string SPEED = "speed"s;
    static inline const std::string DIR = "dir"s;
    static inline const std::string SCORE = "score"s;
    static inline const std::string BAG = "bag"s;
    static inline const std::string LOOT_ID = "id"s;
    static inline const std::string LOOT_TYPE = "type"s;
    static inline const std::string PLAYERS = "players"s;
    static inline const std::string LOST_OBJECTS = "lostObjects"s;
};

struct ConfigFields
{
    static inline const std::string DEFAULT_DOG_SPEED = "defaultDogSpeed"s;
    static inline const std::string DEFAULT_BAG_CAPACITY = "defaultBagCapacity"s;
    static inline const std::string MAPS = "maps"s;
    static inline const std::string LOOT_GENERATOR_CONFIG = "lootGeneratorConfig"s;
    static inline const std::string LOOT_GENERATOR_CONFIG_PERIOD = "period"s;
    static inline const std::string LOOT_GENERATOR_CONFIG_PROBABILILY = "probability"s;
};

namespace boost_json 
{
    std::string GetErrorMes(std::string_view code, std::string_view message);

    struct JoinRequest {
        std::string user_name;
        std::string map_id;   
    };

    JoinRequest ParseJoinRequest(const std::string& object);

    std::string GetMapsJson(const app::list_maps::Result& maps);
    std::string GetMapJson(const app::map_info::Result& map, extra_data::ExtraData ext_data);
    std::string GetGameSateJsonBody(const app::game_state::Result& game_state);
    std::string GetPlayerJsonBody(const app::join_game::Result& player_data);
    std::string GetPlayersJsonBody(const app::players_list::Result& dogs);
    std::string SerializeEmptyJsonObject();


    class JsonValue;
    using ArrayJsonValue = std::vector<JsonValue>;

    class JsonValue {
    public:
        JsonValue(const boost::json::value& value);
        const ArrayJsonValue GetParamAsArray(const std::string& name_array) const;
        std::string GetParamAsString(const std::string& name_param) const;
        int GetParamAsInt(const std::string& name_param) const;
        double GetParamAsDouble(const std::string& name_param) const; 
        JsonValue GetParamAsObj(const std::string& name_param) const; 
        int GetSizeParamArray(const std::string& name_array) const;
        bool ContainsParam(const std::string& name_param) const;
        void SetParentPtr(std::shared_ptr<boost::json::value> ptr);
        boost::json::value GetValue();

    private:
        const boost::json::value& value_;
        std::shared_ptr<boost::json::value> parent_ptr_;
    };

    JsonValue ParseFile(const std::filesystem::path& json_path);
    JsonValue ParseStr(const std::string& str_json);  
} // namespace boost_json 

