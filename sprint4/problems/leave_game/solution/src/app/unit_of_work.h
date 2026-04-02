#pragma once

#include <memory>
#include "../domain/records.h"

namespace app {


enum class TypeOfTransaction{
    Read, Write
};


class UnitOfWorkI {
public:
    virtual void Commit() = 0;
    virtual model::RecordRepositoryI& Records() = 0;
protected:
    ~UnitOfWorkI() = default;
};

class UnitOfWorkFactoryI {
public:
    virtual std::shared_ptr<UnitOfWorkI> CreateUnitOfWork(TypeOfTransaction type_of_tr) = 0;
protected:
    ~UnitOfWorkFactoryI() = default;
};

} // namespace app {