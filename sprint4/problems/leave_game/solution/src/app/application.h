#pragma once
#include "../domain/model.h"
#include "unit_of_work.h"


namespace app {
    
    class Player {
    public:
        using Token = util::Tagged<std::string, Player>;
        using Id = model::Dog::Id;
        Player(Token token, model::Dog* dog, model::GameSession* session) noexcept;
        const Token& GetToken() const noexcept;
        model::Dog& GetDog() const noexcept;
        model::GameSession& GetGameSession() const noexcept;
    private:
        Token token_;
        model::Dog* dog_;
        model::GameSession* session_;
    };

    using ArrPlayersUPtr = std::vector<std::unique_ptr<Player>>;

    class Players {
    public:
        Players() = default;
        // No copy functions.
        Players(const Players&) = delete;
        void operator=(const Players&) = delete;
        
        
        const Player* AddPlayer(model::GameSession* session, model::Dog* dog);
        void AddPlayer(Player&& new_player);
        Player* FindByToken(const Player::Token& token) const noexcept;
        const ArrPlayersUPtr& GetPlayers() const;
        void DeletePlayerForDog(model::Dog* dog);
    private:
        using TokenToPlayer = std::unordered_map<Player::Token, Player*, util::TaggedHasher<Player::Token>>;
        // using DogToPlayer = std::unordered_map<model::Dog*, Player*>;
        ArrPlayersUPtr players_;
        TokenToPlayer token_to_player_;
        // DogToPlayer dog_to_player_;
    
        Player::Token GenerateNewToken();
    
        static const char ZERO_SYMBOL = '0';
        static const unsigned LENGHT_HEX_NUMBER = 16;
    
    
        std::random_device random_device_;
        std::mt19937_64 generator1_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
        }()};
        std::mt19937_64 generator2_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
        }()};
    
    };

    namespace list_maps {

        
        struct MapInfo {
            std::string id;
            std::string name;
        };
        
        using Result = std::vector<MapInfo>;

        class UseCase {
        public:
            explicit UseCase(model::Game& game)
            : game_(game){
            }
            
            const list_maps::Result ListMaps() const;
        private:
            model::Game& game_;
        };

    } // namespace list_maps

    namespace map_info {

        struct Road {
            model::Point start;
            model::Point end;
        };

        struct Building {
            model::Rectangle bounds;
        };

        struct Office {
            std::string id;
            model::Point position;
            model::Offset offset;
        };

        using Roads = std::vector<Road>;
        using Buildings = std::vector<Building>;
        using Offices = std::vector<Office>;

        struct Result {
            explicit Result(const model::Map& map);
            std::string name_map;
            std::string id_map;
            Roads roads_;
            Buildings buildings_;        
            Offices offices_;        
        };

        enum class ErrorReason {
            MapNotFound
        };

        class Error : public std::exception {
        public:
            explicit Error(ErrorReason reason);
            ErrorReason reason_;
        };

        class UseCase {
        public:
            explicit UseCase(model::Game& game);
            const map_info::Result GetMapInfo(const std::string_view map_name);
        private:
            model::Game& game_;
        };

    } // namespace map_info

    namespace join_game {

        struct Result {
            Player::Token token;
            Player::Id player_id;
        };

        enum class ErrorReason {
            InvalidName,
            InvalidMap
        };

        class Error : public std::exception {
        public:
            explicit Error(ErrorReason reason);
            ErrorReason reason_;
        };

        class UseCase {
        public:
            UseCase(model::Game& game, Players& players);
            const Result AddPlayer(const std::string& user_name, const std::string& map_id);
        private:
            model::Game& game_;
            Players& players_;
        };

    } // namespace join_game

    namespace game_state {

        struct LootInBag {
            int id;
            int type;
        };

        struct Dog {
            std::string id;
            model::Pos pos;
            model::Dog::Speed speed;
            std::string dir;
            std::vector<LootInBag> bag;
            int score;
        };

        struct Loot {
            std::string id;
            int type;
            model::Pos pos;
        };

        struct Result {
            std::vector<Dog> dogs;
            std::vector<Loot> loots;
        };

        class UseCase {
        public:
            UseCase();
            const Result GetGameSate(const model::GameSession& game_session);
        };
    } // namespace game_state

    namespace records {

        class UseCase : public model::RetireListenerI {
        public:
            UseCase(Players& players, UnitOfWorkFactoryI* uow_factory);
            const model::Records GetRecords(int start, int max_items) const;
            void OnRetire(model::Dog* dog, std::chrono::milliseconds now) override;
        private:
            UnitOfWorkFactoryI* uow_factory_ = nullptr;
            Players& players_;
        };

    } // namespace records

    namespace players_list {

        struct Dog {
            std::string id;
            std::string name;
        };

        using Result = std::vector<Dog>;

        class UseCase {
        public:
            UseCase();
            const Result GetPlayersListForUser(const model::GameSession::Dogs& dogs);
        };

    } // namespace players_list

    class ApplicationListener {
    public:
        virtual void OnTick(std::chrono::milliseconds time_delta) = 0;
    protected:
        ~ApplicationListener() = default;
    };

    class Application {
    public:
        Application(model::Game& game, UnitOfWorkFactoryI* uow_factory);
        // No copy functions.
        Application(const Application&) = delete;
        void operator=(const Application&) = delete;
    
        Player* FindPlayer(const std::string_view& token) const noexcept;
        const list_maps::Result ListMaps();
        const join_game::Result AddPlayer(const std::string& user_name, const std::string& map_id);
        void AddPlayer(Player&& new_player);
        const map_info::Result GetMapInfo(const std::string_view map_name);
        const game_state::Result GetGameSate(const std::string_view token);
        players_list::Result GetPlayersListForUser(const std::string_view& token);
        void SetDogDirect(const std::string_view& token, const char direct);
        void ChangeGameState(std::chrono::milliseconds time_delta);
        char ConvertDogDirect(const std::string direct);
        void SetListener(ApplicationListener* listener);
        model::Game& GetGameObj() const;
        const ArrPlayersUPtr& GetPlayers() const;
        const model::Records GetRecords(int start, int max_items) const;
        model::RetireListenerI* GetRecordsUseCase();
    private:
        Players players_;
        model::Game& game_;
        join_game::UseCase join_game_;
        list_maps::UseCase list_maps_;
        map_info::UseCase map_info_;
        game_state::UseCase game_state_;
        players_list::UseCase players_list_;
        records::UseCase records_;
        ApplicationListener* listener_ = nullptr;
        UnitOfWorkFactoryI* uow_factory_ = nullptr;

        const model::GameSession::Dogs& GetDogsGameSessionForToken(const std::string_view& token);
    };
    

} // namespace app 
    