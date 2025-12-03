#pragma once
#include <string>
#include <vector>
#include <chrono>

// 储存了实体模型，用于保存数据库对应的映射
namespace model {

    using Timestamp = std::chrono::system_clock::time_point;

    struct User {
        std::string phone;
        std::string username;
        std::string password;
        std::string role;
        double balance = 0.0;
        Timestamp createdAt;
    };

    struct Bill {
        int id = 0;
        std::string description;
        double amount = 0.0;
        Timestamp issueDate;
        Event event;
        Annotation annotation;
    };
    
    // 事项模型，用于保存账单对应的事项
    struct Event {
        int id;
        std::string name;
        Timestamp createdAt;
        EventStatus status;
    };

    // 管理员注解
    struct Annotation {
        int id;
        std::string content;
        int authorid;
    };

    enum EventStatus {
        avaliable, frozen
    };
}