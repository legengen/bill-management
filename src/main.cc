#include "gui/cores/App.h"
#include "data/DatabaseORM.h"
#include "data/UserRepositoryImpl.h"
#include "data/BillRepositoryImpl.h"
#include "data/EventRepositoryImpl.h"
#include "data/AnnotationRepositoryImpl.h"
#include "services/AuthService.h"
#include "services/BillService.h"
#include "services/EventService.h"
#include "services/UserService.h"
#include "services/StatisticsService.h"
#include <iostream>
#include <filesystem>
#include <memory>

int main() {
    try {
        // 1. 确保数据库目录存在
        std::filesystem::create_directories("database");
        
        // 2. 初始化数据库（ORM）
        auto db = std::make_shared<DatabaseORM>("database/bills.db");
        
        // 3. 创建 Repository 实现
        auto user_repo = std::make_shared<UserRepositoryImpl>(db);
        auto bill_repo = std::make_shared<BillRepositoryImpl>(db);
        auto event_repo = std::make_shared<EventRepositoryImpl>(db);
        auto annotation_repo = std::make_shared<AnnotationRepositoryImpl>(db);
        
        // 4. 创建 Service
        auto auth_service = std::make_shared<AuthService>(user_repo);
        auto user_service = std::make_shared<UserService>(user_repo);
        auto bill_service = std::make_shared<BillService>(bill_repo, annotation_repo);
        auto event_service = std::make_shared<EventService>(event_repo);
        auto stats_service = std::make_shared<StatisticsService>(bill_repo);
        
        // 5.  创建并运行应用
        App app(
            auth_service,
            bill_service,
            event_service,
            user_service,
            stats_service
        );
        
        app.Run();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ 程序启动失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}