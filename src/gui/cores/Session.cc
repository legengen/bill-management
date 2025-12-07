#include "Session.h"

void Session::Login(const model::User& user) {
    current_user_ = user;
}

void Session::Logout() {
    current_user_ = std::nullopt;
}