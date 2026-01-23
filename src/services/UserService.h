#pragma once
#include <irepositories.h>

class UserService {
public:
    explicit UserService(std::shared_ptr<repo::IUserRepository> user_repo): user_repository_(user_repo) {}

    std::optional<model::User> GetUser(int user_id);
    std::optional<model::User> GetUserByPhone(const std::string& phone);
    std::vector<model::User> QueryUserByPhone(const std::string& phone);
    std::vector<model::User> GetAllUsers();
    void SetBalance(int user_id, double amount);
private:
    std::shared_ptr<repo::IUserRepository> user_repository_;
};