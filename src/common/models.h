#pragma once
#include <string>
#include <vector>
#include <chrono>

namespace model {

    using Timestamp = std::chrono::system_clock::time_point;

    enum EventStatus {
        Available,
        Frozen
    };

    struct Event {
        int id = 0;
        std::string name;
        Timestamp created_at;
        EventStatus status = EventStatus::Available;
        
        Event() {
            created_at = std::chrono::system_clock::now();
        }
    };

    struct Annotation {
        int id = 0;
        int bill_id = 0;
        std::string content;
        int authorid = 0;
        Timestamp created_at;
        
        Annotation() {
            created_at = std::chrono::system_clock::now();
        }
    };

    struct User {
        int id = 0;
        std::string phone;
        std::string username;
        std::string password;
        std::string role = "user";
        double balance = 0.0;
        Timestamp created_at;
        
        User() {
            created_at = std::chrono::system_clock::now();
        }
        
        User(const std::string& phone, const std::string& username, const std::string& password)
            : phone(phone), username(username), password(password), role("user"), balance(0.0) {
            created_at = std::chrono::system_clock::now();
        }
    };

    struct Bill {
        int id = 0;
        int owner_id = 0;
        int event_id = 0; 
        std::string description;
        double amount = 0.0;
        Timestamp created_at;
        
        Event event;
        Annotation annotation;
        bool has_annotation = false;  // 标记是否有注解
        
        Bill() {
            created_at = std::chrono::system_clock::now();
        }
    };
}