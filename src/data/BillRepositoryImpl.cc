#include "BillRepositoryImpl.h"
#include "irepositories.h"

#include <algorithm>

using namespace orm;

void BillRepositoryImpl::save(const model::Bill& b) {
    auto& storage = db_->GetStorage();
    
    if (b.id == 0) {
        storage.insert(b);
    } else {
        storage.update(b);
    }
}

std::optional<model::Bill> BillRepositoryImpl::findById(int id) {
    auto& storage = db_->GetStorage();

    auto bill = storage.get_optional<model::Bill>(id);
    if (bill.has_value()) {
        auto e = storage.get_optional<model::Event>(bill->event_id);
        if (e.has_value()) {
            bill->event = *e;
        }
    }
    return bill;
}

std::vector<model::Bill> BillRepositoryImpl::queryAll() {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>();
}

std::vector<model::Bill> BillRepositoryImpl::queryByEvent(int ownerId, int eventId) {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>(
        where(c(&model::Bill::owner_id) == ownerId && c(&model::Bill::event_id) == eventId)
    );
}

std::vector<model::Bill> BillRepositoryImpl::queryByEvent(int eventId) {
    auto& storage = db_->GetStorage();
    
    return storage.get_all<model::Bill>(
        where(c(&model::Bill::event_id) == eventId)
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

// ...existing code...

int BillRepositoryImpl::getNextBillId(int owner_id) {
    auto& storage = db_->GetStorage();
    
    // 获取该用户的所有账单，按ID降序排列，取第一个
    auto bills = storage.get_all<model::Bill>(
        where(c(&model::Bill::owner_id) == owner_id),
        order_by(&model::Bill::id).desc(),
        limit(1)
    );
    
    if (bills.empty()) {
        return 1;  // 该用户第一笔账单
    }
    
    return bills[0].id + 1;
}