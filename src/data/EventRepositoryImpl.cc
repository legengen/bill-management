#include "EventRepositoryImpl.h"

using namespace sqlite_orm;

void EventRepositoryImpl::save(const model::Event& e) {
    auto& storage = db_->GetStorage();
    
    if (e.id == 0) {
        // 插入新事件
        model::Event event_copy = e;
        event_copy.id = storage.insert(event_copy);
    } else {
        // 更新现有事件
        storage.update(e);
    }
}

std::optional<model::Event> EventRepositoryImpl::findById(int id) {
    auto& storage = db_->GetStorage();
    
    try {
        auto event = storage.get<model::Event>(id);
        return event;
    } catch (const std::system_error&) {
        // 记录未找到
        return std::nullopt;
    } catch (const std::exception& ex) {
        // 其他错误
        return std::nullopt;
    }
}

std::optional<model::Event> EventRepositoryImpl::findByName(const std::string& name) {
    auto& storage = db_->GetStorage();
    
    try {
        auto events = storage.get_all<model::Event>(
            where(c(&model::Event::name) == name)
        );
        
        if (events.empty()) {
            return std::nullopt;
        }
        
        return events[0];
    } catch (const std::exception& ex) {
        return std::nullopt;
    }
}

bool EventRepositoryImpl::setStatusById(int id, int status) {
    auto& storage = db_->GetStorage();
    
    try {
        // 先查找事件
        auto event_opt = findById(id);
        if (!event_opt.has_value()) {
            return false;
        }
        
        // 更新状态
        auto event = event_opt.value();
        event.status = status;
        storage.update(event);
        
        return true;
    } catch (const std::exception& ex) {
        return false;
    }
}