#include "model_serialization.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp> // для сериализации контейнера vector
#include <boost/serialization/string.hpp> // для сериализации контейнера string
#include <fstream>
#include <filesystem>
using namespace std::literals;

namespace serialization {
    SerializingListener::SerializingListener(std::chrono::milliseconds save_period, const std::string& state_file, app::Application& application)
    : save_period_(save_period)
    , application_(application)
    , state_file_(state_file)
    {}

    void SerializingListener::OnTick(std::chrono::milliseconds time_delta) {
        time_since_save_ += time_delta;
        if (time_since_save_ < save_period_) {
            return;
        }
        SerializeGameState();
        time_since_save_ = 0ms;
    }

    void SerializingListener::SerializeGameState(){
        GameStateRepr game_state_repr;
        const auto& game = application_.GetGameObj();
        const auto& sessions = game.GetSessions();
        // сохраним время игры
        game_state_repr.SetTimeNow(game.GetTimeNow());
        // сохраним собак
        const auto& dogs = game.GetDogs();
        for (const auto& dog : dogs) {
            game_state_repr.AddDogRepr(*dog);
        }
        // сохраним игровые сессии с трофями и списками собак
        for (auto & session : sessions) {  // цикл по списку сессий 
            game_state_repr.AddSessionRepr(*session);
        }
        // сохраним Игроков с токенами, ссылками на собак и ссылками на сессии
        const auto& players = application_.GetPlayers();
        for (const auto& player : players) {
            game_state_repr.AddPlayerRepr(*player);
        }
        // Сохраним game_state_repr в файл
        std::filesystem::path state_file(state_file_);
        std::filesystem::path state_file_tmp = state_file.parent_path() / "state_file_tmp";
        std::ofstream out{state_file_tmp};
        boost::archive::text_oarchive oa{out};
        oa << game_state_repr;
        out.close();
        std::filesystem::rename(state_file_tmp, state_file);
    }
    
    void SerializingListener::RestoreGameState(){
        // Загрузим game_state_repr в файл
        std::ifstream in{state_file_};
        if (!in.is_open()) {
            return;
        }
        boost::archive::text_iarchive ia{in};
        GameStateRepr game_state_repr;
        ia >> game_state_repr;
        in.close();
        auto& game = application_.GetGameObj();
        // Восстанавливааем время
        game.SetTimeNow(game_state_repr.GetTimeNow());
        // Восстанавливаем собак 
        const auto& dogs_repr = game_state_repr.GetDogs();
        for (const auto& dog_r : dogs_repr) {
            game.AddDog(dog_r.Restore());
        };
        // Восстанавливаем игровые сессии
        const auto& sessions_repr = game_state_repr.GetSessions();
        for (const auto& session_r : sessions_repr) {
            game.AddSession(session_r.Restore(game));
        };
        // Восстанавливаем игроков
        const auto& players_repr = game_state_repr.GetPlayers();
        for (const auto& player_r : players_repr) {
            application_.AddPlayer(player_r.Restore(game));
        };
    }

    bool SerializingListener::StateFileNotEmpty() {
        return !state_file_.empty();
    }

    LootsRepr::LootsRepr(const model::Loots& loots){
        loots_ = loots.GetLootsList();
        loots_on_map_count_ = loots.GetCount();
    }

    model::Loots LootsRepr::Restor() const {
        model::Loots loots;
        loots.SetLootsList(loots_);
        return loots;
    }

    DogRepr::DogRepr(const model::Dog& dog)
        : id_(dog.GetId())
        , name_(dog.GetName())
        , pos_(dog.GetPos())
        , speed_(dog.GetSpeed())
        , dir_(dog.GetDir())
        , score_(dog.GetScore())
        , bag_(dog.GetBag()) 
        , enter_time_{dog.GetEnterTime().count()}
        , stop_time_{dog.GetStopTime().count()}
        , stopped_ {dog.GetStoppedPram()}
        {
    }

    model::Dog DogRepr::Restore() const {
        model::Dog dog{name_, id_};
        dog.SetPos(pos_);
        dog.SetDirSpeed(dir_, 0);
        dog.SetSpeed(speed_);
        dog.AddScore(score_);
        dog.SetBag(bag_);
        dog.SetEnterTime(model::Dog::TimePoint(enter_time_));
        dog.SetStopTime(model::Dog::TimePoint(stop_time_));
        dog.SetStoppedPram(stopped_);
        return dog;
    }

    SessionRepr::SessionRepr(const model::GameSession& session, const PointerDogToInd& pointer_dog_to_ind)
    : map_id_(session.GetMap().GetId()) 
    , loots_(session.GetLoots())
    , randomize_spawn_points_(session.GetRandomizeSpawnPoints()) {

        auto& dogs = session.GetDogs();
        for (const auto& dog : dogs) {  // цикл по собакам сессии
            dogs_ind_.push_back(pointer_dog_to_ind.at(dog));    
        }
    }

    model::GameSession SessionRepr::Restore(const model::Game& game) const{
        model::GameSession session{game.FindMap(map_id_), randomize_spawn_points_};
        // Добавляем трофеи
        session.SetLoots(loots_.Restor());
        // Добавляем список ссылок на собак
        const auto& dogs = game.GetDogs();
        for (auto ind : dogs_ind_) {
            session.AddDog(dogs.at(ind).get());
        }
        return session;
    }

    PayerRepr::PayerRepr(const app::Player& player
                        , const PointerDogToInd& pointer_dog_to_ind
                        , const PointerSessionToInd& pointer_Session_to_ind)
    : token_(player.GetToken())
    , dog_ind_(pointer_dog_to_ind.at(&(player.GetDog())))
    , session_ind_(pointer_Session_to_ind.at(&(player.GetGameSession())))
    {}

    app::Player PayerRepr::Restore(const model::Game& game) const {
        const auto& dogs = game.GetDogs();
        auto dog_ptr = dogs.at(dog_ind_).get();
        const auto& session = game.GetSessions();
        auto session_ptr = session.at(session_ind_).get();

        app::Player player{token_, dog_ptr, session_ptr};
        return player;
    }

    void GameStateRepr::AddDogRepr(const model::Dog& dog){
        dogs_.emplace_back(dog);
        pointer_dog_to_ind_[&dog] = dogs_.size() - 1;
    }
    
    void GameStateRepr::AddSessionRepr(const model::GameSession& session){
        sessions_.emplace_back(session, pointer_dog_to_ind_);
        pointer_sesion_to_ind_[&session] = sessions_.size() - 1;
    }
    
    void GameStateRepr::AddPlayerRepr(const app::Player& player){
        players_.emplace_back(player, pointer_dog_to_ind_, pointer_sesion_to_ind_);
    }

    void GameStateRepr::SetTimeNow(std::chrono::milliseconds now){
        time_now_ = now.count();
    }

    const std::vector<DogRepr>& GameStateRepr::GetDogs() const {
        return dogs_;
    }

    const std::vector<SessionRepr>& GameStateRepr::GetSessions() const {
        return sessions_;
    }

    const std::vector<PayerRepr>& GameStateRepr::GetPlayers() const {
        return players_;
    }

    const std::chrono::milliseconds GameStateRepr::GetTimeNow() const {
        return std::chrono::milliseconds{time_now_};
    }

} // namespace serialization