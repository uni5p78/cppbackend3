#include "model.h"
#include "collision_detector.h"
using namespace collision_detector;

#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace model {
using namespace std::literals;

Road::Road(HorizontalTag, Point start, Coord end_x) noexcept
: start_{start}
, end_{end_x, start.y} {
}

Road::Road(VerticalTag, Point start, Coord end_y) noexcept
: start_{start}
, end_{start.x, end_y} {
}

bool Road::IsHorizontal() const noexcept {
    return start_.y == end_.y;
}

bool Road::IsVertical() const noexcept {
    return start_.x == end_.x;
}

Point Road::GetStart() const noexcept {
    return start_;
}

Point Road::GetEnd() const noexcept {
    return end_;
}

Building::Building(Rectangle bounds) noexcept
: bounds_{bounds} {
}

const Rectangle& Building::GetBounds() const noexcept {
    return bounds_;
}

Office::Office(Id id, Point position, Offset offset) noexcept
: id_{std::move(id)}
, position_{position}
, offset_{offset} {
}

const Office::Id& Office::GetId() const noexcept {
    return id_;
}

Point Office::GetPosition() const noexcept {
    return position_;
}

Offset Office::GetOffset() const noexcept {
    return offset_;
}


Map::Map(Id id, std::string name) noexcept
: id_(std::move(id))
, name_(std::move(name)) {
}

const Map::Id& Map::GetId() const noexcept {
    return id_;
}

const std::string& Map::GetName() const noexcept {
    return name_;
}

const Map::Buildings& Map::GetBuildings() const noexcept {
    return buildings_;
}

const Map::Roads& Map::GetRoads() const noexcept {
    return roads_;
}

const Map::Offices& Map::GetOffices() const noexcept {
    return offices_;
}

void Map::AddRoad(const Road&& road) {
    bool vertical = road.IsVertical();
    auto start = road.GetStart();
    auto end = road.GetEnd();
    if(vertical){
        if(start.y > end.y){
            std::swap(start,end); 
        }
        v_paths_.AddPath(start.x, start.y, end.y);
    } else {
        if(start.x > end.x){
            std::swap(start,end); 
        }
        h_paths_.AddPath(start.y, start.x, end.x);
    }
    roads_.emplace_back(road);
}

void Map::AddBuilding(const Building& building) {
    buildings_.emplace_back(building);
}


void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

Pos Map::GetRandomPos() const {
    auto roads = GetRoads();
    if(roads.empty()){
        return {0.0, 0.0};
    }
    std::random_device random_device_;
    std::uniform_int_distribution<int> dist_roads(0, roads.size() - 1);
    int random_road = dist_roads(random_device_);
    auto road = roads[random_road];
    auto start = road.GetStart();
    auto end = road.GetEnd();
    if(road.IsHorizontal()){
        std::uniform_real_distribution<CoordFloat> dist_x(start.x, end.x);
        return {dist_x(random_device_), static_cast<CoordFloat>(start.y)};
    } 
    std::uniform_real_distribution<CoordFloat> dist_y(start.y, end.y);
    return {static_cast<CoordFloat>(start.x), dist_y(random_device_)};
}

Pos Map::GetStartPos() const {
    auto roads = GetRoads();
    if(roads.empty()){
        return {0.0, 0.0};
    }
    auto road = roads[0];
    auto start = road.GetStart();
    return {static_cast<CoordFloat>(start.x), static_cast<CoordFloat>(start.y)};
}

int Map::GetRandomLootType() const {
    std::random_device random_device_;
    std::uniform_int_distribution<int> dist_types(0, loot_types_count_ - 1);
    return dist_types(random_device_);
}


void Map::SetDogSpeed(Dog::Dimension dog_speed){
    dog_speed_ = dog_speed;
}

void Map::SetBagCapacity(int bag_capacity){
    bag_capacity_ = bag_capacity;
}

int Map::GetBagCapacity() const {
    return bag_capacity_;
}


Dog::Dimension Map::GetDogSpeed() const {
    return dog_speed_;
}

void Map::OrderedListPaths::BildListOderedPath(){ 
    auto compare_h = [](const Path left, const Path right){
        return left.level < right.level;
    };
    std::sort(paths_.begin(), paths_.end(), compare_h);
    auto it1 = paths_.begin();
    auto it2 = it1 + 1;
    while (it2 != paths_.end()) {
        // Если второя дорога продолжает первую
        if ((*it1).level == (*it2).level && (*it1).point2 >= (*it2).point1) {
            (*it1).point2 = (*it2).point2; //то первую продолжаем на длину второй
            paths_.erase(it2); // вторую удаляем
            it2 = it1 + 1;
        } else { // если нет - переходим  к сравнению следующей пары
            it1 += 1;
            it2 += 1;
        }
    }
}

void Map::BildListOderedPath(){ 
    h_paths_.BildListOderedPath();
    v_paths_.BildListOderedPath();
}

Dimension Map::GetEndOfPath(bool IsHorizontal, bool to_right, Dimension level_dog, Dimension point_dog) const {
    if (IsHorizontal) {
        return h_paths_.GetEndOfPath(level_dog, point_dog, to_right);
    } 
    return v_paths_.GetEndOfPath(level_dog, point_dog, to_right);
}

Dimension Map::GetEndOfPathV(Dimension level_dog, Dimension point_dog, bool to_up) const {
    return v_paths_.GetEndOfPath(level_dog, point_dog, to_up);
}

void Map::SetLootTypesCount(int count) {
    loot_types_count_ = count;
}

void Map::SetLootsValue(std::vector<int>&& loots_value) {
    loots_value_ = std::move(loots_value);
}

int Map::GetLootValue(int idx) const {
    return loots_value_.at(idx);
}

Dimension Map::OrderedListPaths::GetEndOfPath(Dimension level_dog, Dimension point_dog, bool to_upward_direct) const{
    auto compare_coord = [](Path road, Dimension coord_find){ return road.level < coord_find; };
    auto it = std::lower_bound(paths_.begin(), paths_.end(), level_dog, compare_coord);
    Dimension res = point_dog; // если не найдем дорогу в нужном направлении, то передадим точку собаки

    while (it != paths_.end() && (*it).level == level_dog) { //Если нашли дорогу с нужным уровнем
        auto path = *it;
        if (point_dog >= path.point1 && point_dog <= path.point2) { // если собака на этой дороге
            res =  to_upward_direct ? path.point2 : path.point1; // передаем предел в нужном направлении
            break;
        }
        it += 1;  // переходим на следующую (список отсортирован по возрастаню уровня
                // дороги стоящие "паровозиком" объеденены в один путь)
    }
    return res;
}

void Map::OrderedListPaths::AddPath(Coord coord, Coord mark1, Coord mark2){
    paths_.emplace_back(coord, mark1, mark2);
}


Dog::Dog(std::string user_name, Id id) noexcept
: name_{std::move(user_name)} 
, id_{std::move(id)} 
, enter_time_{Game::GetTimeNow()}
, stop_time_{enter_time_}
{}

const Dog::Id& Dog::GetId() const noexcept {
    return id_;
}

const std::string& Dog::GetName() const noexcept {
    return name_;
}

bool Dog::Speed::IsZero() {
    return dir_x == 0.0 && dir_y == 0.0;
}

void Dog::SetPos(Pos pos){
    pos_ = pos;
}  

void Dog::SetPos(bool IsHorizontal, Pos pos){
    if (IsHorizontal) {
        pos_ = pos;
    } else {
        pos_ = {pos.y, pos.x};
    }
}  

Pos Dog::GetPos() const {
    return pos_;
}  

Dog::Speed Dog::GetSpeed() const {
    return speed_;
}  

void Dog::SetStopParam(bool stop_dog) {
    if (!Dog::stopped_ && stop_dog) {
        stop_time_ = Game::GetTimeNow();
    }
    Dog::stopped_ = stop_dog;
}

void Dog::SetSpeed(Speed new_speed){
    SetStopParam(new_speed.IsZero());
    speed_ = new_speed;
}

void Dog::Stop(){
    SetSpeed({});
}

char Dog::GetDirSymbol() const {
    return static_cast<char>(dir_);
}  

Dog::Dir Dog::GetDir() const {
    return dir_;
}

void Dog::SetDirSpeed(Dir dir, Dimension dog_speed){
    if (dir == Dir::None) { // если направление не указано, останвливаем собаку.
        Stop(); // ориентацию собаки не меняем dir_
        return;
    }
    dir_ = dir;
    if (dir == Dir::Left || dir == Dir::Up) {
        dog_speed = -dog_speed;
    }
    Speed speed{};
    if (dir == Dir::Left || dir == Dir::Right) {
        speed.dir_x = dog_speed;
    } else {
        speed.dir_y = dog_speed;
    }
    SetSpeed(speed);
}

bool Dog::CheckDirSymbol(char dir){
    if(dir != static_cast<char>(Dir::Down) 
    && dir != static_cast<char>(Dir::Left)
    && dir != static_cast<char>(Dir::Right)
    && dir != static_cast<char>(Dir::Up)){
        throw std::invalid_argument("Invalid syntax dog direct.");
    }
    return dir;
}

void Dog::SetNewPosOnRoad(const Map& map, const Milliseconds time_delta){
    auto speed_dir = GetSpeed();  
    if (speed_dir.dir_x == 0.0 && speed_dir.dir_y == 0.0 ) { // Если собака стоит
        return; // то мы ее не трогаем)
    };
    const CoordFloat HalfWideRoad = 0.4;
    auto pos_1 = GetPos();
    auto speed = speed_dir.dir_x;
    bool IsHorizontal = speed != 0.e0;
    CoordFloat level, point1, point2;
    if (IsHorizontal) { // собака движется горизонтально
        level = pos_1.y;
        point1 = pos_1.x;
    } else {  // вертикально)
        speed = speed_dir.dir_y;
        level = pos_1.x;
        point1 = pos_1.y;
    }
    std::chrono::duration<float> time_delta_in_seconds = time_delta;
    point2 = point1 + speed * time_delta_in_seconds.count();  

    int level_int = round(level); 
    int point1_int = round(point1);               
    int point2_int = round(point2); 
    bool to_right = speed>0;
    // Проверка наличия препятствий на пути собаки
    if ((point1_int != point2_int) || (std::abs(point2 - point2_int) >= HalfWideRoad)){
        // Если собака вышла из начаьного кадрата дороги
        // Проверим - есть дальше дорога?)
        auto point_end = map.GetEndOfPath(IsHorizontal, to_right, level_int, point1_int);

        if ((to_right && point2 >= HalfWideRoad + point_end) //если прошли край дороги
        ||   (!to_right && point2 <= -HalfWideRoad + point_end)) {
            CoordFloat minus = to_right ? 1.0 : -1.0;
            point2 = HalfWideRoad * minus + point_end; 
            Stop();  // Останавливаем собаку на краю дороги
        }
    }    
    SetPos(IsHorizontal, {point2, level});
}

const Dog::LootIdList& Dog::GetBag() const {
    return bag_;
}    

void Dog::SetBag(LootIdList bag){
    bag_ = std::move(bag);
}  

void Dog::EmptyBag () {
    bag_.clear();
}

void Dog::AddScore(int score) {
    score_ += score;
}

int Dog::GetScore() const {
    return score_;
}

bool Dog::CheckRetire(Dog::TimePoint now, Milliseconds dog_retirement_time) const {
    return stopped_ && (now - stop_time_) >= dog_retirement_time;
}

void Dog::SetEnterTime(TimePoint time){
    enter_time_ = time;
}

Dog::TimePoint Dog::GetEnterTime()  const{
    return enter_time_;
}

void Dog::SetStopTime(TimePoint time){
    stop_time_ = time;
}

Dog::TimePoint Dog::GetStopTime()  const{
    return stop_time_;
}

void Dog::SetStoppedPram(bool stopped){
    stopped_ = stopped;
}

bool Dog::GetStoppedPram() const{
    return stopped_;
}

GameSession::GameSession(const Map* map, bool randomize_spawn_points) noexcept
: map_(map) 
, randomize_spawn_points_(randomize_spawn_points){
}

void GameSession::AddDog(Dog* dog){
    dogs_.push_back(dog);
}

void GameSession::InitPosAndDirForDog(Dog* dog){
    if (randomize_spawn_points_) {
        dog->SetPos(map_->GetRandomPos());
    } else {
        dog->SetPos(map_->GetStartPos());
    }
    dog->SetDirSpeed(Dog::Dir::Up, 0.0);
}

const GameSession::Dogs& GameSession::GetDogs() const {
    return dogs_;
}

const Map& GameSession::GetMap() const{
    return *map_;
}

void GameSession::AddLoots(int count) {
    for(size_t i = 0; i < count; i++){
        // Вычислить для каждого нового трофея тип и координаты случайным образом
        loots_.AddLoot(Loot{map_->GetRandomLootType(), map_->GetRandomPos()});
    }
}

int GameSession::GetLootsCount() const{
    return loots_.GetCount();
}

const Loots& GameSession::GetLoots() const{
    return loots_;
}

void GameSession::SetLoots(Loots&& new_loots){
    loots_ = std::move(new_loots);
}


bool GameSession::GetRandomizeSpawnPoints() const{
    return randomize_spawn_points_;
}

void GameSession::DeleteDog(Dog* dog) {
    auto it = std::find(dogs_.begin(), dogs_.end(), dog);
    if(it != dogs_.end()) {
        dogs_.erase(it);
    } else {
        throw std::logic_error("Dog not found in Session for RetireDog!");
    }
}

Game::Game(bool randomize_spawn_points)
: randomize_spawn_points_(randomize_spawn_points)
{}


void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

GameSession* Game::GetSession(const Map* map){
    auto& sessions_for_map = sessions_for_map_[map];
    if(sessions_for_map.empty()){ // если пока нет сессий для этой карты
        sessions_.emplace_back(std::make_unique<GameSession>(map, randomize_spawn_points_)); // добавляем сессию для этой карты
        sessions_for_map.push_back(sessions_.back().get()); // добавляем сессию для этой карты
    }
    return sessions_for_map.at(0); // на данном этапе возвращаем "первую"(и пока единственную) сессию для карты 
}

void Game::AddSession(GameSession&& new_session) {
    sessions_.emplace_back(std::make_unique<GameSession>(std::move(new_session))); // добавляем сессию 
    auto* map = &(new_session.GetMap());
    auto& sessions_for_map = sessions_for_map_[map];
    sessions_for_map.push_back(sessions_.back().get()); // добавляем ссылку на сессию в список для этой карты
}

void Game::SetDefaultDogSpeed(Dog::Dimension dog_speed) {
    default_dog_speed_ = dog_speed;
}

void Game::SetDefaultBagCapacity(int bag_capacity) {
    default_bag_capacity_ = bag_capacity;
}

void Game::SetLootGenerator(int period, float probability){
    loot_generator_ = {std::chrono::milliseconds(period), probability};
}

void Game::SetDogRetirementTime(std::chrono::milliseconds dog_retirement_time){
    dog_retirement_time_ = dog_retirement_time;
}

std::chrono::milliseconds Game::GetDogRetirementTime(){
    return dog_retirement_time_;
}


Dog::Dimension Game::GetDefaultDogSpeed() const {
    return default_dog_speed_;
}

int Game::GetDefaultBagCapacity() const {
    return default_bag_capacity_;
}

ItemGathererContaner::Items AddLootsAndOfficesInContaner (const model::GameSession& session) {
    ItemGathererContaner::Items items;
    size_t loot_count = session.GetLootsCount();
    items.reserve(loot_count);
    auto loots = session.GetLoots();
    for(size_t i = 0; i < loot_count; i++) {
        auto loot = loots.GetLoot(i);
        if (loot.status_ == LootStatus::ON_MAP) {
            items.emplace_back(geom::Point2D{loot.pos_.x, loot.pos_.y}, Width::ITEM, ItemType::LOOT, i);
        }
    }
    auto offices = session.GetMap().GetOffices();
    auto offices_count = offices.size();
    items.reserve(items.size() + offices_count);
    for(size_t i = 0; i < offices_count; i++) {
        auto pos = offices.at(i).GetPosition();
        items.emplace_back(geom::Point2D{0.0 + pos.x, 0.0 + pos.y}, Width::OFFICE, ItemType::OFFICE, i);
    }
    return items;
}

// Чтобы добавить трофей в сумку собаки нужно:
// 1. Проверить трофей, что у него статус "на карте", что его еще не забрали
// 2. Проверить, что в сумке есть место
// 3. Положить в сумку трофей (записать его индекс)
// 4. У трофея установить статус "в сумке" 

// Чтобы выгрузить трофеи в офисе:
// 1. У трофеев установить статус "в офисе"
// 2. удалить записи о трофеях из сумки
void CalcLootCollection(const ItemGathererContaner::Gatherers& gatherers, const model::GameSession& session){
    auto items = AddLootsAndOfficesInContaner(session);
    auto gathering_events = FindGatherEvents(ItemGathererContaner{items, gatherers});
    auto dogs = session.GetDogs();
    auto map = session.GetMap();
    int bag_capacity = map.GetBagCapacity();
    for (const auto& event : gathering_events) {
        const auto& dog = dogs.at(event.gatherer_id);
        const auto& item = items.at(event.item_id);
        auto loots = session.GetLoots();
        auto bag = dog->GetBag();

        if(item.type == ItemType::LOOT) { // если item это loot
            if(loots.GetLootStatus(item.id) == LootStatus::ON_MAP && bag.size() < bag_capacity){
                bag.push_back(item.id);
                loots.SetLootStatusById(item.id, LootStatus::IN_BAG);
            }
        } else { // если item это office 
            for(const auto& loot_id : bag) {
                dog->AddScore(map.GetLootValue(loots.GetLoot(loot_id).type_));
                loots.SetLootStatusById(loot_id, LootStatus::IN_OFFICE);
            }
            dog->EmptyBag();
        }
    }
}



void Game::ChangeGameState(std::chrono::milliseconds time_delta){
    for (auto & session : sessions_) {  // цикл по списку сессий
        // у всех собак изменить координаты и скорость в соответствии с движением во времени
        auto& dogs = session->GetDogs();
        ItemGathererContaner::Gatherers gatherers;
        gatherers.reserve(dogs.size());
        auto* map = &(session->GetMap());
        IncreaseNowTimer(time_delta);
        auto now = GetTimeNow();
        auto dog_retirement_time = GetDogRetirementTime();
        std::vector<Dog*> retire_dogs;
        for (auto& dog : dogs) {  // цикл по собакам сессии
            auto old_pos = dog->GetPos();
            dog->SetNewPosOnRoad(*map, time_delta);
            auto new_pos = dog->GetPos();
            gatherers.emplace_back(geom::Point2D{old_pos.x, old_pos.y}, geom::Point2D{new_pos.x, new_pos.y}, Width::DOG);
            if(dog->CheckRetire(now, dog_retirement_time)) {
                retire_dogs.push_back(dog);
            };
        }
        // Вычисляем сбор трофеев
        CalcLootCollection(gatherers, *session);
        // Удалаяем из игры игроков и их собак, которые долго стоят
        for (auto dog : retire_dogs) {
            retire_listener_->OnRetire(dog, now); // удаляем игрока и делаем запись рекорда в базу
            session->DeleteDog(dog); // удаляем указатель на собаку в игровой сессии карты
            this->DeleteDog(dog); // game Удаляем собаку из игры
        }
        // Вычислить количество трофеев, которое следует добавить на карту сессии
        int NewLootCount = loot_generator_.Generate(time_delta, session->GetLootsCount(), dogs.size());
        // Добвляем трофеи с случайными типами в случайные места на дорогах
        session->AddLoots(NewLootCount);
    }
}

const Game::SissionPointers& Game::GetSessions() const {
    return sessions_;
}

const Game::DogPointers& Game::GetDogs() const {
    return dogs_;
}

void Game::SetRetireListener(RetireListenerI* listener){
    retire_listener_ = listener;
}

std::chrono::milliseconds Game::GetTimeNow() {
    return model::Game::now_;
}

void Game::SetTimeNow(std::chrono::milliseconds now) {
    now_ = now;
}

void Game::IncreaseNowTimer(std::chrono::milliseconds time_delta) {
    now_ += time_delta;
}

void Game::DeleteDog(Dog* dog){
    auto it = std::find_if(dogs_.begin(), dogs_.end()
    , [dog](auto& dog_ptr){return dog_ptr.get() == dog;});
    if(it != dogs_.end()) {
        dogs_.erase(it);
    } else {
        throw std::logic_error("Dog not found in Game for RetireDog!");
    }
}

const Game::Maps& Game::GetMaps() const noexcept {
    return maps_;
}

const Map* Game::FindMap(const Map::Id& id) const noexcept {
    if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
        return &maps_.at(it->second);
    }
    return nullptr;
}

Dog* Game::AddDog(std::string name){
    Dog::Id id(dogs_.size() + 1);
    dogs_.push_back(std::make_unique<Dog>(name, id));
    return dogs_.back().get();
}

void Game::AddDog(Dog&& dog){
    dogs_.push_back(std::make_unique<Dog>(std::move(dog)));
}

int Loots::GetCount() const {
    return loots_on_map_count_;
}

void Loots::AddLoot(Loot loot) {
    bool insert = false;
    size_t id = 0;
    for(; id < loots_.size(); id++){
        if(loots_[id].status_ == LootStatus::IN_OFFICE) {
            loots_[id] = loot;
            insert = true;
            break;
        }
    }
    if(!insert) {
        loots_.push_back(loot);
        id = loots_.size() - 1;
    }
    SetLootStatusById(id, LootStatus::ON_MAP);
}

Loot Loots::GetLoot(size_t idx) {
    return loots_.at(idx);
}


const Loots::LootsVector& Loots::GetLootsList() const {
    return loots_;
}

void Loots::SetLootsList(LootsVector loots_arr){
    loots_ = std::move(loots_arr);
    loots_on_map_count_ = std::count_if(loots_.begin(), loots_.end()
    , [](const auto& loot){
        return loot.status_ == LootStatus::ON_MAP;
    });
}


void Loots::SetLootStatusById(size_t id, LootStatus status) {
    auto& loot_status_ = loots_[id].status_;
    if (loot_status_ == status) {
        return;
    }
    if (status == LootStatus::ON_MAP) {
        loots_on_map_count_++;        
    } else if (loot_status_  == LootStatus::ON_MAP) {
        loots_on_map_count_--;
    }    
    loot_status_ = status;
}

LootStatus Loots::GetLootStatus(size_t id) const{
    return loots_.at(id).status_;
}


}  // namespace model
