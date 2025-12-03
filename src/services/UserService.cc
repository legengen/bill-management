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

std::vector<model::User> UserService::QueryUserByPhone(const std::string& phone){
    if (phone.empty()) {
        return std::vector<model::User>();
    }
    auto users = user_repository_->queryByPhonePartial(phone);
    return users;
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