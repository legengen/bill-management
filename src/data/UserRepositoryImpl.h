#pragma once
#include "irepositories.h"
#include "DatabaseORM.h"
#include <memory>

class UserRepositoryImpl : public repo::IUserRepository {
public:
    explicit UserRepositoryImpl(std::shared_ptr<DatabaseORM> db) : db_(db) {}
    
    void save(const model::User& u) override;
    std::optional<model::User> findById(int id) override;
    std::optional<model::User> queryByPhone(const std::string& phone) override;
    std::vector<model::User> queryByPhonePartial(const std::string& partial) override;
    bool setBalanceByPhone(const std::string& phone, double balance) override;
    
private:
    std::shared_ptr<DatabaseORM> db_;
};