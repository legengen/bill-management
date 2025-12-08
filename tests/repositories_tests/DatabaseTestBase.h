#pragma once
#include <gtest/gtest.h>
#include "DatabaseORM.h"
#include "UserRepositoryImpl.h"
#include "BillRepositoryImpl.h"
#include "EventRepositoryImpl.h"
#include "AnnotationRepositoryImpl.h"
#include <memory>

class DatabaseTestBase : public ::testing::Test {
protected:
    void SetUp() override {
        // 使用内存数据库（每个测试独立）
        db_ = std::make_shared<DatabaseORM>(":memory:");
        
        // 创建 Repository 实现
        user_repo_ = std::make_shared<UserRepositoryImpl>(db_);
        bill_repo_ = std::make_shared<BillRepositoryImpl>(db_);
        event_repo_ = std::make_shared<EventRepositoryImpl>(db_);
        annotation_repo_ = std::make_shared<AnnotationRepositoryImpl>(db_);
        
        // 插入测试种子数据
        SeedTestData();
    }

    void TearDown() override {
        annotation_repo_. reset();
        event_repo_.reset();
        bill_repo_.reset();
        user_repo_.reset();
        db_.reset();
    }

    // 插入测试数据
    void SeedTestData() {
        // 创建测试用户
        model::User user1;
        user1.phone = "13800000001";
        user1.username = "TestUser1";
        user1.password = "password123";
        user1.role = "user";
        user1.balance = 1000.0;
        user_repo_->save(user1);
        
        model::User user2;
        user2.phone = "13800000002";
        user2.username = "TestAdmin";
        user2.password = "admin123";
        user2.role = "admin";
        user2.balance = 5000.0;
        user_repo_->save(user2);
        
        // 创建测试事件
        model::Event event1;
        event1.name = "餐饮";
        event1.status = model::EventStatus::Available;
        event_repo_->save(event1);
        
        model::Event event2;
        event2.name = "交通";
        event2.status = model::EventStatus::Available;
        event_repo_->save(event2);
        
        model::Event event3;
        event3.name = "购物";
        event3.status = model::EventStatus::Frozen;
        event_repo_->save(event3);
    }

    // 辅助函数
    model::User CreateUser(const std::string& phone, 
                           const std::string& username,
                           const std::string& password = "test123") {
        model::User user;
        user.phone = phone;
        user.username = username;
        user.password = password;
        user.role = "user";
        user.balance = 0.0;
        return user;
    }
    
    model::Bill CreateBill(int owner_id, int event_id, double amount,
                           const std::string& desc = "Test Bill") {
        model::Bill bill;
        bill.owner_id = owner_id;
        bill.event_id = event_id;
        bill.amount = amount;
        bill.description = desc;
        return bill;
    }
    
    model::Event CreateEvent(const std::string& name, 
                             int status = model::EventStatus::Available) {
        model::Event event;
        event.name = name;
        event.status = status;
        return event;
    }
    
    model::Annotation CreateAnnotation(int bill_id, int author_id,
                                       const std::string& content = "Test Note") {
        model::Annotation annotation;
        annotation.bill_id = bill_id;
        annotation.authorid = author_id;
        annotation.content = content;
        return annotation;
    }

    // 共享资源
    std::shared_ptr<DatabaseORM> db_;
    std::shared_ptr<UserRepositoryImpl> user_repo_;
    std::shared_ptr<BillRepositoryImpl> bill_repo_;
    std::shared_ptr<EventRepositoryImpl> event_repo_;
    std::shared_ptr<AnnotationRepositoryImpl> annotation_repo_;
};