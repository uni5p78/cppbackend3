#include "db.h"
#include <string_view>



namespace db {
    using namespace std::literals;
    using pqxx::operator"" _zv;

    RecordRepositoryImpl::RecordRepositoryImpl(pqxx::work* work, pqxx::read_transaction* read_transaction)
    : work_(work)
    , read_transaction_(read_transaction){
    }
    

    const model::Records RecordRepositoryImpl::GetRecords(int start, int max_items) const {
        model::Records res;
        auto query_text = "SELECT name, score, playTime FROM retired_players"
        " ORDER BY score Desc, playTime, name ;";
        // Выполняем запрос и итерируемся по строкам ответа
        for (auto [name, score, play_time] : read_transaction_->query<std::string_view, int, double>(query_text)) {
            res.emplace_back(std::string(name), score, play_time);
        }
        return res;
    }

    void RecordRepositoryImpl::SaveRecord(std::string name, int score, double play_time_seconds){
        work_->exec_params(
            R"(
            INSERT INTO retired_players (name, score, playTime) VALUES ($1, $2, $3)
            )"_zv,
            name, score, play_time_seconds);
    }


    UnitOfWorkImpl::UnitOfWorkImpl(ConnectionPool::ConnectionWrapper&& connection
    , app::TypeOfTransaction type_of_tr) 
        : connection_(std::move(connection)) {
        if (type_of_tr == app::TypeOfTransaction::Read) {
            read_transaction_ = std::make_unique<pqxx::read_transaction>(*connection_);
        } else {
            work_ = std::make_unique<pqxx::work>(*connection_);
        }
        record_ = std::make_unique<RecordRepositoryImpl>(work_.get(), read_transaction_.get());
    }

    void UnitOfWorkImpl::Commit() {
        work_->commit();
    }

    model::RecordRepositoryI& UnitOfWorkImpl::Records() {
        return *record_;
    }

    UnitOfWorkFactoryImpl::UnitOfWorkFactoryImpl(ConnectionPool& connection_pool) 
    : connection_pool_(connection_pool){
    }

    std::shared_ptr<app::UnitOfWorkI> UnitOfWorkFactoryImpl::CreateUnitOfWork(app::TypeOfTransaction type_of_tr) {
        auto connection = connection_pool_.GetConnection();
        return std::make_shared<UnitOfWorkImpl>(std::move(connection), type_of_tr);
    }

    Database::Database(std::string db_url, unsigned connection_count)
    : connection_pool_(ConnectionPool(connection_count, [db_url]{return std::make_shared<pqxx::connection>(db_url);}))
    , uow_factory_(UnitOfWorkFactoryImpl(connection_pool_)) {
        auto connection = connection_pool_.GetConnection();
        pqxx::work work{*connection};
        work.exec(R"(
            CREATE TABLE IF NOT EXISTS retired_players (
            id  SERIAL PRIMARY KEY,
            name varchar(100) NOT NULL,
            score integer NOT NULL,
            playTime FLOAT4 NOT NULL
            );)"_zv
        );
        work.exec(R"(
            CREATE INDEX IF NOT EXISTS retired_players_score_idx on retired_players 
            (score Desc, playTime, name);
            )"_zv
        );
        work.commit();
    }

    app::UnitOfWorkFactoryI* Database::GetUnitOfWorkFactory(){
        return &uow_factory_;
    }

} // namespace db