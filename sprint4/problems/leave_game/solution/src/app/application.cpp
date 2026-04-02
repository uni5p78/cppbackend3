#include "application.h"

using std::literals::string_literals::operator""s;
using std::literals::string_view_literals::operator""sv;

namespace app {            
    const char ZERO_CHAR = 0;
    
    Player::Player(Token token, model::Dog* dog, model::GameSession* session) noexcept
        : token_(std::move(token)) 
        , dog_(dog) 
        , session_(session) 
    {}

    const Player::Token& Player::GetToken() const noexcept{
        return token_;
    }

    model::Dog& Player::GetDog() const noexcept{
        return *dog_;
    }

    model::GameSession& Player::GetGameSession() const noexcept{
        return *session_;
    }

    const Player* Players::AddPlayer(model::GameSession* session, model::Dog* dog){
        Player::Token token = GenerateNewToken();
        players_.push_back(std::make_unique<Player>(token, dog, session));
        auto player_ptr = players_.back().get();
        token_to_player_[token] = player_ptr;
        return player_ptr;
    }

    void Players::AddPlayer(Player&& new_player) {
        auto token = new_player.GetToken();
        players_.push_back(std::make_unique<Player>(std::move(new_player)));
        token_to_player_[token] = players_.back().get();
    }

    Player* Players::FindByToken(const Player::Token& token) const noexcept {
        if (auto it = token_to_player_.find(token); it != token_to_player_.end()) {
            return it->second;
        }
        return nullptr;
    }

    const ArrPlayersUPtr& Players::GetPlayers() const {
        return players_;
    }

    void Players::DeletePlayerForDog(model::Dog* dog) {
        auto it = std::find_if(players_.begin(), players_.end()
        , [dog](const auto& player){return dog == &(player->GetDog());});
        if (it != players_.end()) {
            token_to_player_.erase((*it)->GetToken());
            players_.erase(it);
        }
    }

    Player::Token Players::GenerateNewToken(){
        std::stringstream stream;
        // токен это строка из 32 символов полученная из двух 
        // 64-битных псевдо-случайных чисел в hex-представлении
        // переведенных в строки по 16 символов с ведущими нулями
        stream << std::hex << std::right << std::setfill(ZERO_SYMBOL) 
            << std::setw(LENGHT_HEX_NUMBER) << generator1_() 
            << std::setw(LENGHT_HEX_NUMBER) << generator2_();
        return Player::Token(stream.str());     
    }

    namespace list_maps {
      
        const list_maps::Result UseCase::ListMaps() const {
            auto maps = game_.GetMaps();
            list_maps::Result res(maps.size());
            size_t i = 0;
            for(const auto& map : maps){
                res[i].id = *map.GetId();
                res[i].name = map.GetName();
                ++i;
            };
            return res;           
        }

    } // namespace list_maps

    namespace map_info {
        Error::Error(ErrorReason reason)
        : reason_(reason){
        }

        Result::Result(const model::Map& map)
        : id_map(*map.GetId())
        , name_map(map.GetName()){
            const auto buildings = map.GetBuildings();
            size_t buildings_count = buildings.size();
            buildings_.resize(buildings_count);
            for(size_t i=0; i<buildings_count; ++i){
                buildings_[i].bounds = buildings[i].GetBounds();
            }

            const auto roads = map.GetRoads();
            size_t roads_count = roads.size();
            roads_.resize(roads_count);
            for(size_t i=0; i<roads_count; ++i){
                roads_[i].start = roads[i].GetStart();
                roads_[i].end = roads[i].GetEnd();
            }

            const auto offices = map.GetOffices();
            size_t offices_count = offices.size();
            offices_.resize(offices_count);
            for(size_t i=0; i<offices_count; ++i){
                offices_[i].id = *offices[i].GetId();
                offices_[i].offset = offices[i].GetOffset();
                offices_[i].position = offices[i].GetPosition();
            }
        }

        UseCase::UseCase(model::Game& game)
        : game_(game){
        }


        const map_info::Result UseCase::GetMapInfo(const std::string_view map_name){
            std::string map_id = std::string(map_name);
            auto map = game_.FindMap(model::Map::Id{map_id});
    
            if (!map) {
                throw map_info::Error(map_info::ErrorReason::MapNotFound);
            }
            return map_info::Result(*map);
        }

    } // namespace map_info


    namespace join_game {
    
        Error::Error(ErrorReason reason)
        : reason_(reason){
        }

        UseCase::UseCase(model::Game& game, Players& players)
        : game_(game)
        , players_(players){

        }

        const join_game::Result UseCase::AddPlayer(const std::string& user_name, const std::string& map_id) {
            if(user_name.empty()){
                throw join_game::Error{join_game::ErrorReason::InvalidName};
            }
            auto map = game_.FindMap(model::Map::Id{map_id});
            if (!map){
                throw join_game::Error{join_game::ErrorReason::InvalidMap};
            } 
            auto dog = game_.AddDog(user_name); // Создаем собаку для игрока
            auto session = game_.GetSession(map); // находим сессию для карты, которую запросил игрок
            session->AddDog(dog); // добавляем собаку в сессию игры
            session->InitPosAndDirForDog(dog); // Вычисляем для собаки начальные координаты и направление движения       
            auto new_player = players_.AddPlayer(session, dog); // создаем игрока
        
            return {new_player->GetToken(), new_player->GetDog().GetId()};
        }
    
    } // namespace join_game
        
    namespace game_state {

        UseCase::UseCase() {}

        std::vector<LootInBag> GetBag(model::Dog* const dog, const model::Loots::LootsVector& loots) {
            auto loot_id_list = dog->GetBag();
            std::vector<LootInBag> res;
            res.reserve(loot_id_list.size());
            for (auto loot_id : loot_id_list) {
                res.emplace_back(loot_id, loots.at(loot_id).type_);
            };
            return res;
        }

        const Result UseCase::GetGameSate(const model::GameSession& game_session){
            Result res;
            const auto loots = game_session.GetLoots().GetLootsList();
            const auto dogs = game_session.GetDogs();
            res.dogs.reserve(dogs.size());
            for(const auto& dog : dogs){
                res.dogs.push_back({});
                auto& rec_dog = res.dogs.back();
                rec_dog.id = std::to_string(*dog->GetId());
                rec_dog.pos = dog->GetPos();
                rec_dog.speed = dog->GetSpeed();
                rec_dog.dir = dog->GetDirSymbol();
                rec_dog.bag = GetBag(dog, loots);
                rec_dog.score = dog->GetScore();
            }

            int loots_size = loots.size();
            res.loots.reserve(loots_size);
            for (int i = 0; i < loots_size; i++) {
                const auto& loot = loots.at(i);
                if (loot.status_ != model::LootStatus::ON_MAP) { // если трофей уже подобран и удален их списка 
                    break; // пропускаем его
                }
                res.loots.push_back({});
                auto& rec_loot = res.loots.back();
                rec_loot.id = std::to_string(i);
                rec_loot.type = loot.type_;
                rec_loot.pos = loot.pos_;
            }
            return res;
        }
    }   // namespace game_state 

    namespace records {
        UseCase::UseCase(Players& players, UnitOfWorkFactoryI* uow_factory) 
        : players_{players}
        , uow_factory_{uow_factory}
        {
            if (!uow_factory_) {
                throw std::logic_error("uow_factory_ == nullptr (UseCase::UseCase(Players& players, UnitOfWorkFactoryI* uow_factory))"); 
            }
        }

        const model::Records UseCase::GetRecords(int start, int max_items) const{
            auto uo_work = uow_factory_->CreateUnitOfWork(TypeOfTransaction::Read);
            return uo_work->Records().GetRecords(start, max_items);
        }

        void UseCase::OnRetire(model::Dog* dog, std::chrono::milliseconds now) {
            // Удалить игрока собаки
            players_.DeletePlayerForDog(dog);
            // Сделать запись в таблице достижений
            auto uo_work = uow_factory_->CreateUnitOfWork(TypeOfTransaction::Write);
            auto play_time = now - dog->GetStopTime();
            double play_time_seconds = 1.0 * play_time.count() / 1000.0;
            uo_work->Records().SaveRecord(dog->GetName(), dog->GetScore(), play_time_seconds);
            uo_work->Commit();
        }
        
    } // namespace records
        
    namespace players_list {

        UseCase::UseCase() {}

        const Result UseCase::GetPlayersListForUser(const model::GameSession::Dogs& dogs){
            Result res;
            res.reserve(dogs.size());
            for(const auto& dog : dogs){
                res.emplace_back(std::to_string(*dog->GetId()), dog->GetName());
            }
            return res;
        }
    }   // namespace players_list 


    Application::Application(model::Game& game, UnitOfWorkFactoryI* uow_factory) 
    : game_(game)
    , join_game_(game, players_)
    , list_maps_(game)
    , map_info_(game)
    , uow_factory_(uow_factory)
    , records_(players_, uow_factory)
    {
    }
    
    Player* Application::FindPlayer(const std::string_view& token) const noexcept {
        return players_.FindByToken(Player::Token(std::string(token)));
    }

    const list_maps::Result Application::ListMaps() {
        return list_maps_.ListMaps();
    }

    const join_game::Result Application::AddPlayer(const std::string& user_name, const std::string& map_id) {
        return join_game_.AddPlayer(user_name, map_id);
    }

    void Application::AddPlayer(Player&& new_player){
        players_.AddPlayer(std::move(new_player));
    }

    const map_info::Result Application::GetMapInfo(const std::string_view map_name){
        return map_info_.GetMapInfo(map_name);
    }
        
    const game_state::Result Application::GetGameSate(const std::string_view token){
        return game_state_.GetGameSate(FindPlayer(token)->GetGameSession());
    }

    players_list::Result Application::GetPlayersListForUser(const std::string_view& token){
        return players_list_.GetPlayersListForUser(GetDogsGameSessionForToken(token));
    }

    void Application::SetDogDirect(const std::string_view& token, const char direct){
        const auto player = FindPlayer(token);
        const auto dog_speed = player->GetGameSession().GetMap().GetDogSpeed();
        auto dir = static_cast<model::Dog::Dir>(direct);
        auto& dog = player->GetDog();
        dog.SetDirSpeed(dir, dog_speed);
    }

    void Application::ChangeGameState(std::chrono::milliseconds time_delta){
        game_.ChangeGameState(time_delta);
        if (listener_) {
            listener_->OnTick(time_delta);
        }
    }

    char Application::ConvertDogDirect(const std::string direct){
        size_t ch_count = direct.size();
        if (ch_count == 0) {
            return ZERO_CHAR;
        }
        if (ch_count == 1 && model::Dog::CheckDirSymbol(direct[0])) {
            return direct[0];
        }
        throw std::invalid_argument("Invalid syntax dog direct.");
    }

    void Application::SetListener(ApplicationListener* listener){
        listener_ = listener;
    }
        
    model::Game& Application::GetGameObj() const {
        return game_;
    }

    const ArrPlayersUPtr& Application::GetPlayers() const {
        return players_.GetPlayers();
    }

    const model::Records Application::GetRecords(int start, int max_items) const {
        return records_.GetRecords(start, max_items);
    }

    model::RetireListenerI* Application::GetRecordsUseCase() {
        return &records_;
    }


    const model::GameSession::Dogs& Application::GetDogsGameSessionForToken(const std::string_view& token){
        return FindPlayer(token)->GetGameSession().GetDogs();
    }
    
} // namespace app 
    