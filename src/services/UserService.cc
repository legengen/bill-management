#include "UserService.h"

std::optional<model::User> UserService::GetUser(int user_id){
    if(user_id <= 0) {
        return std::nullopt;
    }
    auto user = user_repository_->findById(user_id);
    if (!user.has_value()) {
        return std::nullopt;
    }
    return user;
}

std::optional<model::User> UserService::GetUserByPhone(const std::string& phone) {
    return user_repository_->queryByPhone(phone);
}

std::vector<model::User> UserService::QueryUserByPhone(const std::string& phone){
    auto users = user_repository_->queryByPhonePartial(phone);
    return users;
}

std::vector<model::User> UserService::GetAllUsers() {
    return user_repository_->queryAll();  // 获取全部
}

void UserService::SetBalance(int user_id, double amount){
    if (user_id <= 0) {
        return;
    } else if (amount < 0) {
        return;
    }

    auto user = user_repository_->findById(user_id);
    if (!user.has_value()) {
        return;
    }

    auto u = user.value();
    u.balance = amount;
    user_repository_->save(u);
}