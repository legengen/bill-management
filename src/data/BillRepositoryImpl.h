#pragma once
#include <irepositories.h>
#include "DatabaseORM.h"

class BillRepositoryImpl : public repo::IBillRepository {
public:
    explicit BillRepositoryImpl(std::shared_ptr<DatabaseORM> db) : db_(db) {}
    void save(const model::Bill& b);

    std::optional<model::Bill> findById(int id);

    std::vector<model::Bill> queryByEvent(int ownerId, int eventId);
    std::vector<model::Bill> queryByEvent(std::string& name); // 仅管理员可用

    std::vector<model::Bill> queryByTime(int ownerId, model::Timestamp from, model::Timestamp to);
    std::vector<model::Bill> queryByTime(model::Timestamp from, model::Timestamp to); // 仅管理员可用

    std::vector<model::Bill> queryByPhone(const std::string& phone); // 仅管理员可用

    std::vector<model::Bill> queryByTimeInOrder(model::Timestamp from, model::Timestamp to); // 仅管理员可用
    std::vector<model::Bill> queryByTimeAndEventInOrder(model::Timestamp from, model::Timestamp to); // 仅管理员可用
private:
    std::shared_ptr<DatabaseORM> db_;
};
