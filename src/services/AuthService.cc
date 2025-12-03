#include "AuthService.h"

std::optional<model::User> AuthService::Login(const std::string& phone, const std::string& password){
    std::optional<model::User> user = user_repository_->queryByPhone(phone);
    if (!user.has_value() || user.value().password != password) {
        return std::nullopt;
    }
    return user;
}

std::optional<model::User> AuthService::Register(const std::string& phone, const std::string& username, const std::string& password){
    std::optional<model::User> user = user_repository_->queryByPhone(phone);
    if (user.has_value()) {
        return std::nullopt;
    } else if (phone.empty() || username.empty() || password.empty()) {
        return std::nullopt;
    } 
    model::User u(phone, username, password);
    user_repository_->save(u);
    return u;
}

bool AuthService::ResetPassword(int userId, const std::string& oldPwd, const std::string& newPwd){
    std::optional<model::User> user = user_repository_->findById(userId);
    if (user.value().password != oldPwd) {
        return false;
    }
    user.value().password = newPwd;
    return true;
}