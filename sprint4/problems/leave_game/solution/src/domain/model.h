#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <sstream>
#include <memory>
#include <iomanip>
#include <chrono>
#include <algorithm>

#include "tagged.h"
#include "loot_generator.h"   

namespace model {

struct Width
{
    static inline const double ITEM = 0.0;
    static inline const double DOG = 0.6;
    static inline const double OFFICE = 0.5;
};

enum class LootStatus {ON_MAP, IN_BAG, IN_OFFICE};

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

using CoordFloat = double;
struct Pos {
    CoordFloat x{}, y{};
}; 

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept;
    Road(VerticalTag, Point start, Coord end_y) noexcept;
    bool IsHorizontal() const noexcept;
    bool IsVertical() const noexcept;
    Point GetStart() const noexcept;
    Point GetEnd() const noexcept;
private:
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept;
    const Rectangle& GetBounds() const noexcept;
private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;
    Office(Id id, Point position, Offset offset) noexcept;
    const Id& GetId() const noexcept;
    Point GetPosition() const noexcept;
    Offset GetOffset() const noexcept;
private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map;

class Dog {
    public:
        using Id = util::Tagged<int, Dog>;
        using LootIdList = std::vector<size_t>;
        using TimePoint = std::chrono::milliseconds;
        using Milliseconds = std::chrono::milliseconds;
        Dog(std::string user_name, Id id) noexcept;
        const Id& GetId() const noexcept;
        const std::string& GetName() const noexcept;
    
        using Dimension = CoordFloat;
        struct Speed {
            Dimension dir_x{}, dir_y{};
            bool IsZero();
        };    
        enum class Dir: char {Left = 'L', Right = 'R', Up = 'U', Down = 'D', None = 0}; 
    
        void SetPos(Pos pos);
        void SetPos(bool IsHorizontal, Pos pos);
        Pos GetPos() const;
        Speed GetSpeed() const;
        void SetStopParam(bool stop_dog);
        void SetSpeed(Speed new_speed);
        void Stop();
        char GetDirSymbol() const;
        Dir GetDir() const;
        void SetDirSpeed(Dir dir, Dimension dog_speed);
        static bool CheckDirSymbol(char dir);
        void SetNewPosOnRoad(const Map& map, const Milliseconds time_delta); 
        const LootIdList& GetBag() const;   
        void SetBag(LootIdList bag);   
        void EmptyBag();
        void AddScore(int score); 
        int GetScore() const;
        bool CheckRetire(TimePoint now, Milliseconds dog_retirement_time) const;
        void SetEnterTime(TimePoint time);
        TimePoint GetEnterTime() const;
        void SetStopTime(TimePoint time);
        TimePoint GetStopTime() const;
        void SetStoppedPram(bool stopped);
        bool GetStoppedPram() const;
    private:
        Id id_;
        std::string name_;
        Pos pos_;
        Speed speed_;
        Dir dir_ = Dir::Up;
        LootIdList bag_;
        int score_ = 0;
        TimePoint enter_time_{};
        TimePoint stop_time_{};
        bool stopped_ = true;
};

class RecordsRepositary {
public:
    virtual void Save(const Dog& dog) = 0;
private:
    ~RecordsRepositary() = default;
};
        
class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept;
    const Id& GetId() const noexcept;
    const std::string& GetName() const noexcept;
    const Buildings& GetBuildings() const noexcept;
    const Roads& GetRoads() const noexcept;
    const Offices& GetOffices() const noexcept;
    void AddRoad(const Road&& road);
    void AddBuilding(const Building& building);
    void AddOffice(Office office);
    Pos GetRandomPos() const;
    Pos GetStartPos() const;
    int GetRandomLootType() const;
    void SetDogSpeed(Dog::Dimension dog_speed);
    void SetBagCapacity(int bag_capacity);
    int GetBagCapacity() const;
    Dog::Dimension GetDogSpeed() const;
    void BildListOderedPath();
    Dimension GetEndOfPath(bool IsHorizontal, bool to_right, Dimension level_dog, Dimension point_dog) const;
    Dimension GetEndOfPathV(Dimension level_dog, Dimension point_dog, bool to_right) const;
    void SetLootTypesCount(int count);
    void SetLootsValue(std::vector<int>&& loots_value); 
    int GetLootValue(int idx) const; 
private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;
    struct Path {Coord level, point1, point2;};
    // Coord level - это координа общая для обеих точек отрезка дороги 
    // для горизонтальных дорог это "y", для вертикальных - "x"
    // point1, point2 - это координаты точек на оставшейся оси, отличной от level
    class OrderedListPaths {
    public:
        void BildListOderedPath();
        Dimension GetEndOfPath(Dimension level_dog, Dimension point_dog, bool to_right) const;
        void AddPath(Coord base, Coord point1, Coord point2);
    private:
        std::vector<Path> paths_;
    };
    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
    Dog::Dimension dog_speed_;
    OrderedListPaths h_paths_;
    OrderedListPaths v_paths_;
    int loot_types_count_;
    int bag_capacity_;
    std::vector<int> loots_value_;
};

struct Loot {
    int type_;
    Pos pos_;  
    LootStatus status_ = LootStatus::IN_OFFICE;   
};

class Loots {
public:
    using LootsVector = std::vector<Loot>;
    using LootId = int;
    Loots() = default;

    int GetCount() const;
    void AddLoot(Loot);
    Loot GetLoot(size_t idx);
    const LootsVector& GetLootsList() const;
    void SetLootsList(LootsVector loots_arr);
    void SetLootStatusById(size_t id, LootStatus status);
    LootStatus GetLootStatus(size_t id) const;
private:
    LootsVector loots_;
    int loots_on_map_count_ = 0;
};

class GameSession {
public:
    using Dogs = std::vector<Dog*>;
    GameSession(const Map* map, bool randomize_spawn_points) noexcept;
    void AddDog(Dog* dog);
    void InitPosAndDirForDog(Dog* dog);
    const Dogs& GetDogs() const;
    const Map& GetMap() const;
    void AddLoots(int count);
    int GetLootsCount() const;
    const Loots& GetLoots() const;
    void SetLoots(Loots&&);
    bool GetRandomizeSpawnPoints() const; 
    void DeleteDog(Dog* dog);
private:
    Dogs dogs_;
    const Map* map_;
    bool randomize_spawn_points_;
    Loots loots_;
};

class RetireListenerI {
public:
    virtual void OnRetire(Dog* dog, std::chrono::milliseconds now) = 0;
protected:
    ~RetireListenerI() = default;
};

class Game {
public:
    using Maps = std::vector<Map>;
    using SissionPointers = std::vector<std::unique_ptr<GameSession>>;
    using MapToSessions = std::unordered_map<const Map*, std::vector<GameSession*>>;
    using DogPointers = std::vector<std::unique_ptr<Dog>>;
    Game(bool randomize_spawn_points);
    void AddMap(Map map);
    const Maps& GetMaps() const noexcept;
    const Map* FindMap(const Map::Id& id) const noexcept;
    // No copy functions.
    Game(const Game&) = delete;
    void operator=(const Game&) = delete;
    Dog* AddDog(std::string name);
    void AddDog(Dog&& dog);
    GameSession* GetSession(const Map* map);
    void AddSession(GameSession&& new_session);
    void SetDefaultDogSpeed(Dog::Dimension dog_speed);
    void SetDefaultBagCapacity(int bag_capacity);
    void SetLootGenerator(int period, float probability);
    void SetDogRetirementTime(std::chrono::milliseconds dog_retirement_time);
    std::chrono::milliseconds GetDogRetirementTime();
    Dog::Dimension GetDefaultDogSpeed() const;
    int GetDefaultBagCapacity() const;
    void ChangeGameState(std::chrono::milliseconds time_delta);
    const SissionPointers& GetSessions() const;
    const DogPointers& GetDogs() const;
    void SetRetireListener(RetireListenerI* listener);
    static std::chrono::milliseconds GetTimeNow();
    static void SetTimeNow(std::chrono::milliseconds now);
private:
    void IncreaseNowTimer(std::chrono::milliseconds time_delta);
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    DogPointers dogs_;
    SissionPointers sessions_;
    MapToSessions sessions_for_map_;
    Dog::Dimension default_dog_speed_ = 1.0;
    int default_bag_capacity_ = 3;
    bool randomize_spawn_points_;
    loot_gen::LootGenerator loot_generator_;
    std::chrono::milliseconds dog_retirement_time_;
    RetireListenerI* retire_listener_ = nullptr;
    static inline std::chrono::milliseconds now_{};
    void DeleteDog(Dog* dog);
};

}  // namespace model
