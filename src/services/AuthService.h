#pragma once
#include "../data/irepositories.h"

class AuthService {
public:
    explicit AuthService(std::shared_ptr<repo::IUserRepository> repo):
        user_repository_(std::move(repo)) {}

    std::optional<model::User> Login(const std::string& phone, const std::string& password);
    std::optional<model::User> Register(const std::string& phone, const std::string& username, const std::string& password);
    bool ResetPassword(int userId, const std::string& oldPwd, const std::string& newPwd);

private:
    std::shared_ptr<repo::IUserRepository> user_repository_;
};