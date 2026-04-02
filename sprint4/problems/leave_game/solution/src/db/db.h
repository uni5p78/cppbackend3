#pragma once

#include <memory>
#include "connection_pool.h"
#include "../app/unit_of_work.h"
#include <pqxx/zview.hxx>

namespace db {

class RecordRepositoryImpl : public model::RecordRepositoryI {
public:
    explicit RecordRepositoryImpl(pqxx::work* work, pqxx::read_transaction* read_transaction);
    const model::Records GetRecords(int start, int max_items) const override;
    void SaveRecord(std::string name, int score, double play_time_seconds) override;
private:
    pqxx::work* work_;
    pqxx::read_transaction* read_transaction_;
};

class UnitOfWorkImpl : public app::UnitOfWorkI {
public:
    explicit UnitOfWorkImpl(ConnectionPool::ConnectionWrapper&& connection, app::TypeOfTransaction type_of_tr);
    void Commit() override;
    model::RecordRepositoryI& Records() override;
private:
    ConnectionPool::ConnectionWrapper connection_;
    std::unique_ptr<pqxx::work> work_;
    std::unique_ptr<pqxx::read_transaction> read_transaction_;
    std::unique_ptr<model::RecordRepositoryI> record_;
};


class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactoryI {
public:
    explicit UnitOfWorkFactoryImpl(ConnectionPool& connection_pool);
    std::shared_ptr<app::UnitOfWorkI> CreateUnitOfWork(app::TypeOfTransaction type_of_tr) override;
private:
    ConnectionPool& connection_pool_;
};


class Database {
public:
    explicit Database(std::string db_url, unsigned connection_count);
    app::UnitOfWorkFactoryI* GetUnitOfWorkFactory();
private:
    ConnectionPool connection_pool_;
    UnitOfWorkFactoryImpl uow_factory_; 
}; 

} // namespace db 