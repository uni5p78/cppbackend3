#pragma once
#include <boost/serialization/vector.hpp>

#include "../app/application.h"
#include "../domain/model.h"
#include "geom.h"
#include <string>

namespace model {

    template <typename Archive>
void serialize(Archive& ar, Pos& pos, [[maybe_unused]] const unsigned version) {
    ar&(pos.x);
    ar&(pos.y);
}

template <typename Archive>
void serialize(Archive& ar, Loot& loot, [[maybe_unused]] const unsigned version) {
    ar&(loot.status_);
    ar&(loot.type_);
    ar&(loot.pos_);
}
template <typename Archive>
void serialize(Archive& ar, model::Dog::Speed& speed, [[maybe_unused]] const unsigned version) {
    ar&(speed.dir_x);
    ar&(speed.dir_y);
}
}  // namespace model

namespace serialization {

class SerializingListener: public app::ApplicationListener {
public:
    SerializingListener(std::chrono::milliseconds save_period, const std::string& state_file, app::Application& application);
    void OnTick(std::chrono::milliseconds time_delta) override;
    void SerializeGameState();
    void RestoreGameState();
    bool StateFileNotEmpty();
private:
    std::chrono::milliseconds time_since_save_;
    std::chrono::milliseconds save_period_;
    app::Application& application_;
    const std::string& state_file_;
};

class LootsRepr {
public:
    LootsRepr() = default;
    explicit LootsRepr(const model::Loots& loots);
    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version);
    model::Loots Restor() const;
private:
    std::vector<model::Loot> loots_;
    int loots_on_map_count_ = 0;
};

// DogRepr (DogRepresentation) - сериализованное представление класса Dog
class DogRepr {
public:
    DogRepr() = default;
    explicit DogRepr(const model::Dog& dog);

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version);
    model::Dog Restore() const;
private:
    model::Dog::Id id_ = model::Dog::Id{0u};
    std::string name_;
    model::Pos pos_;
    model::Dog::Speed speed_;
    model::Dog::Dir dir_ = model::Dog::Dir::Up;
    int score_ = 0;
    model::Dog::LootIdList bag_;
    long int enter_time_{};
    long int stop_time_{};
    bool stopped_ = true;

};

using PointerDogToInd = std::unordered_map<const model::Dog*, size_t>;
using PointerSessionToInd = std::unordered_map<const model::GameSession*, size_t>;

class SessionRepr {
public:
    SessionRepr() = default;

    explicit SessionRepr(const model::GameSession& session, const PointerDogToInd& pointer_dog_to_ind);

    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version);
    model::GameSession Restore(const model::Game& game) const;

private:
    std::vector<size_t> dogs_ind_;
    model::Map::Id map_id_ = model::Map::Id{""};
    LootsRepr loots_;
    bool randomize_spawn_points_ = true;
};

class PayerRepr {
public:
    PayerRepr() = default;
    explicit PayerRepr(const app::Player& player
        , const PointerDogToInd& pointer_dog_to_ind
        , const PointerSessionToInd& pointer_Session_to_ind);
    template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version);
    app::Player Restore(const model::Game& game) const;
private:
    app::Player::Token token_ = app::Player::Token{""};
    size_t dog_ind_ = 0;
    size_t session_ind_ = 0;
};

class GameStateRepr {
public:
    GameStateRepr() = default;
    void AddDogRepr(const model::Dog& dog);
    void AddSessionRepr(const model::GameSession& session);
    void AddPlayerRepr(const app::Player& player);
    void SetTimeNow(std::chrono::milliseconds now) ;

        template <typename Archive>
    void serialize(Archive& ar, [[maybe_unused]] const unsigned version);
    const std::vector<DogRepr>& GetDogs() const;
    const std::vector<SessionRepr>& GetSessions() const;
    const std::vector<PayerRepr>& GetPlayers() const;
    const std::chrono::milliseconds GetTimeNow() const;
private:
    std::vector<DogRepr> dogs_;
    PointerDogToInd pointer_dog_to_ind_; 
    std::vector<SessionRepr> sessions_;
    PointerSessionToInd pointer_sesion_to_ind_; 
    std::vector<PayerRepr> players_;
    long int time_now_{};
};




    template <typename Archive>
    void LootsRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& loots_;
        ar& loots_on_map_count_;
    }

    template <typename Archive>
    void DogRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar&* id_;
        ar& name_;
        ar& pos_;
        ar& speed_;
        ar& dir_;
        ar& score_;
        ar& bag_;
        ar& enter_time_;
        ar& stop_time_;
        ar& stopped_;
    }

    template <typename Archive>
    void SessionRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& dogs_ind_;
        ar& *map_id_;
        ar& loots_;
        ar& randomize_spawn_points_;
    }

    template <typename Archive>
    void PayerRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& *token_;
        ar& dog_ind_;
        ar& session_ind_;
    }

    template <typename Archive>
    void GameStateRepr::serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
        ar& dogs_;
        ar& sessions_;
        ar& players_;
        ar& time_now_;
    }

}  // namespace serialization


namespace geom {

template <typename Archive>
void serialize(Archive& ar, Point2D& point, [[maybe_unused]] const unsigned version) {
    ar& point.x;
    ar& point.y;
}

template <typename Archive>
void serialize(Archive& ar, Vec2D& vec, [[maybe_unused]] const unsigned version) {
    ar& vec.x;
    ar& vec.y;
}

}  // namespace geom

