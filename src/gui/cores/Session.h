#pragma once
#include "models.h"
#include <optional>
#include <memory>

class Session {
public:
    static Session& Instance() {
        static Session instance;
        return instance;
    }
    
    void Login(const model::User& user);
    void Logout();
    
    bool IsLoggedIn() const { return current_user_. has_value(); }
    bool IsAdmin() const { return IsLoggedIn() && current_user_->role == "admin"; }
    
    std::optional<model::User> GetCurrentUser() const { return current_user_; }
    int GetUserId() const { return current_user_ ?  current_user_->id : 0; }
    std::string GetUsername() const { return current_user_ ? current_user_->username : ""; }
    
private:
    Session() = default;
    std::optional<model::User> current_user_;
};