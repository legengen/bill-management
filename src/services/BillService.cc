#include <BillService.h>

std::optional<model::Bill> BillService::CreateBill(int owner_id, model::Bill data) {
    last_error_.clear();

    auto user = user_repository_->findById(owner_id);
    if (!user.has_value()) {
        last_error_ = "用户不存在";
        return std::nullopt;
    }

    double new_balance = user->balance - data.amount;
    if (new_balance < 0) {
        last_error_ = "余额不足，当前余额: " + std::to_string(user->balance);
        return std::nullopt;
    }

    data.owner_id = owner_id;
    bill_repository_->save(data);

    user_repository_->setBalanceByPhone(user->phone, new_balance);

    return data;
}

std::vector<model::Bill> BillService::QueryByTime(int owner_id, model::Timestamp from, model::Timestamp to) {
    if (owner_id <= 0) {
        return bill_repository_->queryByTime(from, to);
    }

    if (from > to) {
        return std::vector<model::Bill>();
    }

    return bill_repository_->queryByTime(owner_id, from, to);
}

std::vector<model::Bill> BillService::queryByEvent(int owner_id, int event_id) {
    if (owner_id <= 0 || event_id <= 0) {
        return bill_repository_->queryByEvent(event_id);
    }
    return bill_repository_->queryByEvent(owner_id, event_id);
}

std::vector<model::Bill> BillService::queryByPhone(std::string phone) {
    if (phone.empty()) {
        return bill_repository_->queryAll();
    }
    return bill_repository_->queryByPhone(phone);
}

void BillService::editBill(int bill_id, model::Bill updates) {
    if (bill_id <= 0) {
        return;
    }
    
    auto existing = bill_repository_->findById(bill_id);
    if (!existing.has_value()) {
        return;
    }
    
    updates.id = bill_id;
    updates.owner_id = existing->owner_id;
    bill_repository_->save(updates);
}

void BillService::deleteBill(int bill_id) {
    if (bill_id <= 0) {
        return;
    }

    auto bill = bill_repository_->findById(bill_id);
    if (!bill.has_value()) {
        return ;
    }
    bill_repository_->remove(bill_id);
}

void BillService::annotateBill(int bill_id, model::Annotation a) {
    if (bill_id <= 0) {
        return;
    }
    
    auto bill = bill_repository_->findById(bill_id);
    if (!bill.has_value()) {
        return;
    }

    if (a.content.empty() || a.content == "") {
        return;
    }

    annotation_repository_->save(a);
    
    bill->annotation = a;
    bill->has_annotation = true;
    bill_repository_->save(*bill);
}

int BillService::GetNextBillId() {
    return bill_repository_->getNextBillId();
}