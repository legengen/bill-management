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

    return db_->GetStorage().get_optional<model::User>(where(c(&model::User::phone) == phone));
}

std::vector<model::User> UserRepositoryImpl::queryByPhonePartial(const std::string& partial) {
    if (partial.empty()) {
        return {};
    }

    return db_->GetStorage().get_all<model::User>(where(like(&model::User::phone, partial)));
}

bool UserRepositoryImpl::setBalanceByPhone(const std::string& phone) {
    if (phone.empty()) {
        return false;
    }

    auto user = db_->GetStorage().get<model::User>(where(c(&model::User::phone) = phone));
    db_->GetStorage().update(user);
}