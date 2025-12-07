#pragma once
#include <string>
#include <vector>
#include <chrono>

namespace model {

    using Timestamp = int64_t;

    inline Timestamp Now() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }
    
    // 辅助函数：时间戳转 time_point
    inline std::chrono::system_clock::time_point ToTimePoint(Timestamp ts) {
        return std::chrono::system_clock::time_point(std::chrono::seconds(ts));
    }
    
    // 辅助函数：time_point 转时间戳
    inline Timestamp FromTimePoint(std::chrono::system_clock::time_point tp) {
        return std::chrono::duration_cast<std::chrono::seconds>(
            tp. time_since_epoch()
        ).count();
    }

    namespace EventStatus {
        constexpr int Available = 0;
        constexpr int Frozen = 1;
    }

    struct Event {
        int id = 0;
        std::string name;
        Timestamp created_at;
        int status = EventStatus::Available;
        
        Event() {
            created_at = Now();
        }
    };

    struct Annotation {
        int id = 0;
        int bill_id = 0;
        std::string content;
        int authorid = 0;
        Timestamp created_at;
        
        Annotation() {
            created_at = Now();
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
            created_at = Now();
        }
        
        User(const std::string& phone, const std::string& username, const std::string& password)
            : phone(phone), username(username), password(password), role("user"), balance(0.0) {
            created_at = Now();
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
            created_at = Now();
        }
    };
}