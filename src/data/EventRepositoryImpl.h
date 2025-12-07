#pragma once
#include "irepositories.h"
#include "DatabaseORM.h"
#include <memory>

class EventRepositoryImpl : public repo::IEventRepository {
public:
    explicit EventRepositoryImpl(std::shared_ptr<DatabaseORM> db) : db_(db) {}
    
    void save(const model::Event& e) override;
    std::optional<model::Event> findById(int id) override;
    std::optional<model::Event> findByName(const std::string& name) override;
    bool setStatusById(int id, int status) override;
    
private:
    std::shared_ptr<DatabaseORM> db_;
};