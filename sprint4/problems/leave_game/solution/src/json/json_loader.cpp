#include "json_loader.h"
#include "json.h"


#include <filesystem>
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace std::literals;

namespace json_loader {

void AddRoads(model::Map& map, const json_w::ArrayJsonValue& roads){
    for(const auto& road : roads){
        bool horizontal_road = road.ContainsParam(RoadCoord::END_X);
        if (horizontal_road){
            model::Road road_model(model::Road::HORIZONTAL
            , {road.GetParamAsInt(RoadCoord::START_X), road.GetParamAsInt(RoadCoord::START_Y)}
            , road.GetParamAsInt(RoadCoord::END_X));
            map.AddRoad(move(road_model));
        } else {
            model::Road road_model(model::Road::VERTICAL
            , {road.GetParamAsInt(RoadCoord::START_X), road.GetParamAsInt(RoadCoord::START_Y)}
            , road.GetParamAsInt(RoadCoord::END_Y));
            map.AddRoad(move(road_model));
        }
    }
    map.BildListOderedPath();
}

void AddBuildings(model::Map& map, const json_w::ArrayJsonValue& buildings){
    for(const auto& building : buildings){
        model::Building building_model(
            { {building.GetParamAsInt(BuildingFields::POSITION_X), building.GetParamAsInt(BuildingFields::POSITION_Y)}
            , {building.GetParamAsInt(BuildingFields::WIDTH), building.GetParamAsInt(BuildingFields::HEIGHT)}}
        );
        map.AddBuilding(move(building_model));
    }
}

void AddOffices(model::Map& map, const json_w::ArrayJsonValue& offices){
    using IdOffice = util::Tagged<std::string, model::Office>;
    for(const auto& office : offices){
        std::string id =  office.GetParamAsString(OfficeFields::ID);
        model::Office office_model(IdOffice(id)
            , {office.GetParamAsInt(OfficeFields::POSITION_X), office.GetParamAsInt(OfficeFields::POSITION_Y)}
            , {office.GetParamAsInt(OfficeFields::OFFSET_X), office.GetParamAsInt(OfficeFields::OFFSET_Y)}
        );
        map.AddOffice(move(office_model));
    }
}


void SetDogSpeed(model::Map& map, const model::Game& game, const json_w::JsonValue& jmap){
    // Если для карты есть скорость записываем ее в модель
    if(jmap.ContainsParam(MapFields::DOG_SPEED)){
        map.SetDogSpeed(jmap.GetParamAsDouble(MapFields::DOG_SPEED));
    } else { // если нет - записываем скорость по умолчанию для всех карт
        map.SetDogSpeed(game.GetDefaultDogSpeed());
    }
}

void SetBagCapacity(model::Map& map, const model::Game& game, const json_w::JsonValue& jmap){
    if(jmap.ContainsParam(MapFields::BAG_CAPACITY)){ //если есть
        map.SetBagCapacity(jmap.GetParamAsInt(MapFields::BAG_CAPACITY));
    } else { 
        map.SetBagCapacity(game.GetDefaultBagCapacity());
    }

}

void SetLootTypes(model::Map& map, const model::Game& game, const json_w::JsonValue& jmap
, extra_data::ExtraData& ext_data){
    // Записываем количество типов трофеев для карты
    if(!jmap.ContainsParam(MapFields::LOOT_TYPES)){
        throw std::logic_error("No Loot types for map in config");
    } 
    auto jloots = jmap.GetParamAsArray(MapFields::LOOT_TYPES);
    std::vector<int> loots_value;  
    for (const auto& jloot : jloots) {
        loots_value.push_back(jloot.GetParamAsInt(MapFields::LOOT_VALUE));
    }
    map.SetLootsValue(std::move(loots_value)); 
    map.SetLootTypesCount(jloots.size());
    // список типов трофеев для карты для передачи фронтэнду
    // сохраняем не в модель - в отдельное место
    ext_data.SetLootTypesForMap(map.GetId(), jmap.GetParamAsObj(MapFields::LOOT_TYPES).GetValue());
}


void SetParamsForGameModel(model::Game& game, const json_w::JsonValue& json_obj) {
    // Устанавливаем скорость собак по умолчаниюю для всех карт
    if(json_obj.ContainsParam(ConfigFields::DEFAULT_DOG_SPEED)){ //если есть
        game.SetDefaultDogSpeed(json_obj.GetParamAsDouble(ConfigFields::DEFAULT_DOG_SPEED));
    } else { // если нет - записываем скорость по умолчанию 1.0
        game.SetDefaultDogSpeed(1.0);
    }
    
    // Устанавливаем вместимость рюкзака по умолчаниюю для всех карт
    if(json_obj.ContainsParam(ConfigFields::DEFAULT_BAG_CAPACITY)){ //если есть
        game.SetDefaultBagCapacity(json_obj.GetParamAsDouble(ConfigFields::DEFAULT_BAG_CAPACITY));
    } else { // если нет - записываем вместимость по умолчанию 3
        game.SetDefaultBagCapacity(3);
    }
    
    // Устанавливаем параметры генератора количества потеряных вещей
    if(json_obj.ContainsParam(ConfigFields::LOOT_GENERATOR_CONFIG)){ //если есть
    auto loot_gen = json_obj.GetParamAsObj(ConfigFields::LOOT_GENERATOR_CONFIG);
        game.SetLootGenerator(
            loot_gen.GetParamAsDouble(ConfigFields::LOOT_GENERATOR_CONFIG_PERIOD)
          , loot_gen.GetParamAsDouble(ConfigFields::LOOT_GENERATOR_CONFIG_PROBABILILY));
    } 
    
    // Устанавливаем время простоя собаки, после которого игрока исключают из игры
    if(json_obj.ContainsParam(ConfigFields::DOG_RETIREMENT_TIME)){ //если есть
        auto time_Double = json_obj.GetParamAsDouble(ConfigFields::DOG_RETIREMENT_TIME)*1000;
        std::chrono::milliseconds time{int(time_Double)};
        game.SetDogRetirementTime(time);
    } else {
        game.SetDogRetirementTime(std::chrono::minutes(1));
    }

}


void LoadGame(model::Game& game, const std::filesystem::path& json_path, extra_data::ExtraData& ext_data) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    using IdMap = util::Tagged<std::string, model::Map>;
    const auto json_obj = json_w::ParseFile(json_path);
    SetParamsForGameModel(game, json_obj);
    
    const auto jmaps = json_obj.GetParamAsArray(ConfigFields::MAPS);
    // В цикле добавляем карты в модель игры
    for(const auto& jmap : jmaps){
        model::Map map(IdMap(jmap.GetParamAsString(MapFields::ID))
                           , jmap.GetParamAsString(MapFields::NAME));
        AddRoads(map, jmap.GetParamAsArray(MapFields::ROADS));
        AddBuildings(map, jmap.GetParamAsArray(MapFields::BILDINGS));
        AddOffices(map, jmap.GetParamAsArray(MapFields::OFFICES));
        SetDogSpeed(map, game, jmap);
        SetLootTypes(map, game, jmap, ext_data);
        SetBagCapacity(map, game, jmap);
        game.AddMap(move(map)); // Добавляем карту
    }
}

}  // namespace json_loader

