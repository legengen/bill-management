#include "BillRepositoryImpl.h"
#include "irepositories.h"

#include <algorithm>

using namespace orm;

void BillRepositoryImpl::save(const model::Bill& b) {
    auto& storage = db_->GetStorage();
    
    if (b.id == 0) {
        model::Bill bill_copy = b;
        bill_copy.id = storage.insert(bill_copy);
    } else {
        storage.update(b);
    }
}

std::optional<model::Bill> BillRepositoryImpl::findById(int id) {
    auto& storage = db_->GetStorage();
    
    try {
        auto bill = storage.get<model::Bill>(id);
        
        // 加载关联的 Event
        try {
            bill. event = storage.get<model::Event>(bill.event_id);
        } catch (...) {
            // Event 不存在，使用默认值
        }
        
        return bill;
    } catch (const std::system_error&) {
        return std::nullopt;
    }
}

std::vector<model::Bill> BillRepositoryImpl::queryByEvent(int ownerId, int eventId) {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>(
        where(c(&model::Bill::owner_id) == ownerId && c(&model::Bill::event_id) == eventId)
    );
}

std::vector<model::Bill> BillRepositoryImpl::queryByEvent(std::string& name) {
    auto& storage = db_->GetStorage();
    
    // 先查找事件
    auto events = storage.get_all<model::Event>(
        where(c(&model::Event::name) == name)
    );
    
    if (events.empty()) {
        return {};
    }
    
    int event_id = events[0].id;
    
    return storage.get_all<model::Bill>(
        where(c(&model::Bill::event_id) == event_id)
    );
}

std::vector<model::Bill> BillRepositoryImpl::queryByTime(int ownerId, 
                                                          model::Timestamp from, 
                                                          model::Timestamp to) {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>(
        where(
            c(&model::Bill::owner_id) == ownerId &&
            c(&model::Bill::created_at) >= from &&
            c(&model::Bill::created_at) <= to
        )
    );
}

std::vector<model::Bill> BillRepositoryImpl::queryByTime(model::Timestamp from, 
                                                          model::Timestamp to) {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>(
        where(
            c(&model::Bill::created_at) >= from &&
            c(&model::Bill::created_at) <= to
        )
    );
}

std::vector<model::Bill> BillRepositoryImpl::queryByTimeInOrder(model::Timestamp from, 
                                                                 model::Timestamp to) {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>(
        where(
            c(&model::Bill::created_at) >= from &&
            c(&model::Bill::created_at) <= to
        ),
        order_by(&model::Bill::created_at).asc()
    );
}

std::vector<model::Bill> BillRepositoryImpl::queryByTimeAndEventInOrder(model::Timestamp from, 
                                                                         model::Timestamp to) {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>(
        where(
            c(&model::Bill::created_at) >= from &&
            c(&model::Bill::created_at) <= to
        ),
        multi_order_by(
            order_by(&model::Bill::created_at).asc(),
            order_by(&model::Bill::event_id).asc()
        )
    );
}

std::vector<model::Bill> BillRepositoryImpl::queryByPhone(const std::string& phone) {
    auto& storage = db_->GetStorage();
    
    // 先查找用户
    auto users = storage.get_all<model::User>(
        where(c(&model::User::phone) == phone)
    );
    
    if (users.empty()) {
        return {};
    }
    
    int user_id = users[0].id;
    
    return storage.get_all<model::Bill>(
        where(c(&model::Bill::owner_id) == user_id)
    );
}

void BillRepositoryImpl::remove(int id) {
    auto& storage = db_->GetStorage();
    
    try {
        storage.remove<model::Bill>(id);
    } catch (const std::exception& e) {
        // 可选：记录日志或忽略
    }
}