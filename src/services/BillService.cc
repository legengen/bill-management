#include <BillService.h>

std::optional<model::Bill> BillService::CreateBill(int owner_id, model::Bill data) {
    if (owner_id <= 0) {
        return std::nullopt;
    }

    if (data.amount <= 0.0) {
        return std::nullopt;
    } 

    data.owner_id = owner_id;
    bill_repository_->save(data);
    return data;
}

std::vector<model::Bill> BillService::QueryByTime(int owner_id, model::Timestamp from, model::Timestamp to) {
    if (owner_id <= 0) {
        return {};
    }

    if (from > to) {
        return std::vector<model::Bill>();
    }

    return bill_repository_->queryByTime(owner_id, from, to);
}

std::vector<model::Bill> BillService::queryByEvent(int owner_id, int event_id) {
    if (owner_id <= 0 || event_id <= 0) {
        return {};
    }
    return bill_repository_->queryByEvent(owner_id, event_id);
}

std::vector<model::Bill> BillService::queryByPhone(std::string phone) {
    if (phone.empty()) {
        return {};
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