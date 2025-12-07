#pragma once
#include "Router.h"
#include "Session.h"
#include "AuthService.h"
#include "BillService.h"
#include "EventService.h"
#include "UserService.h"
#include "StatisticsService.h"
#include <memory>

class App {
public:
    App(std::shared_ptr<AuthService> auth_service,
        std::shared_ptr<BillService> bill_service,
        std::shared_ptr<EventService> event_service,
        std::shared_ptr<UserService> user_service,
        std::shared_ptr<StatisticsService> stats_service);
    
    void Run();
    
    // 服务访问器
    AuthService& GetAuthService() { return *auth_service_; }
    BillService& GetBillService() { return *bill_service_; }
    EventService& GetEventService() { return *event_service_; }
    UserService& GetUserService() { return *user_service_; }
    StatisticsService& GetStatisticsService() { return *stats_service_; }
    
    static App& Instance() { return *instance_; }
    
private:
    void RegisterScreens();
    
    std::shared_ptr<AuthService> auth_service_;
    std::shared_ptr<BillService> bill_service_;
    std::shared_ptr<EventService> event_service_;
    std::shared_ptr<UserService> user_service_;
    std::shared_ptr<StatisticsService> stats_service_;
    
    static App* instance_;
};