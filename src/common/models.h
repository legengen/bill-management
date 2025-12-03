#pragma once
#include <string>
#include <vector>
#include <chrono>

// 储存了实体模型，用于保存数据库对应的映射
namespace model {

    using Timestamp = std::chrono::system_clock::time_point;

    enum EventStatus {
        avaliable, frozen
    };

    // 事项模型，用于保存账单对应的事项
    struct Event {
        int id;
        std::string name;
        Timestamp created_at;
        EventStatus status;
        Event() = default;
    };

    // 管理员注解
    struct Annotation {
        int id;
        std::string content;
        int authorid;
        Annotation() = default;
    };

    struct User {
        int id = 0;
        std::string phone;
        std::string username;
        std::string password;
        std::string role;
        double balance = 0.0;
        Timestamp created_at;
        User() : id(0), phone(""), username(""), password("") {}
        User(const std::string& phone, const std::string& username, const std::string& password): 
            phone(phone), username(username), password(password){}
    };

    struct Bill {
        int id = 0;
        int owner_id = 0;
        std::string description;
        Timestamp created_at;
        Event event;
        Annotation annotation;
        double amount = 0.0;
        Bill() {
            created_at = std::chrono::system_clock::now();
        };
    };
}