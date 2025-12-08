#include "UserRepositoryImpl.h"

using namespace orm;

void UserRepositoryImpl::save(const model::User& u) {
    if (u.id == 0) {
        db_->GetStorage().insert(u);
    } else {
        db_->GetStorage().update(u);
    }    
}

std::optional<model::User> UserRepositoryImpl::findById(int id) {
    if (id <= 0) {
        return std::nullopt;
    } 

    return db_->GetStorage().get_optional<model::User>(id);
}

std::optional<model::User> UserRepositoryImpl::queryByPhone(const std::string& phone) {
    if (phone.empty()) {
        return std::nullopt;
    }

    auto users = db_->GetStorage().get_all_optional<model::User>(where(c(&model::User::phone) == phone));
    return users.empty() ? std::nullopt : users[0];
}

std::vector<model::User> UserRepositoryImpl::queryByPhonePartial(const std::string& partial) {
    if (partial.empty()) {
        return {};
    }

    return db_->GetStorage().get_all<model::User>(where(like(&model::User::phone, "%" + partial + "%")));
}

bool UserRepositoryImpl::setBalanceByPhone(const std::string& phone, double balance) {
    if (phone.empty()) {
        return false;
    }

    auto users = db_->GetStorage().get_all_optional<model::User>(where(c(&model::User::phone) == phone));
    if (users.empty()) {
        return false;
    }
    users[0]->balance = balance;
    db_->GetStorage().update(*users[0]);
    return true;
}