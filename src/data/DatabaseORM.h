#pragma once
#include "../common/models.h"
#include <sqlite_orm/sqlite_orm.h>
#include <memory>

namespace orm = sqlite_orm;

inline auto CreateStorage(const std::string& db_path) {
    using namespace sqlite_orm;
    
    return make_storage(
        db_path,

        make_table("users",
            make_column("id", &model::User::id, primary_key(). autoincrement()),
            make_column("phone", &model::User::phone, unique()),
            make_column("username", &model::User::username),
            make_column("password", &model::User::password),
            make_column("role", &model::User::role, default_value("user")),
            make_column("balance", &model::User::balance, default_value(0.0)),
            make_column("created_at", &model::User::created_at)
        ),

        make_table("events",
            make_column("id", &model::Event::id, primary_key().autoincrement()),
            make_column("name", &model::Event::name, unique()),
            make_column("status", &model::Event::status, default_value(model::EventStatus::Available)),
            make_column("created_at", &model::Event::created_at)
        ),

        make_table("bills",
            make_column("id", &model::Bill::id, primary_key().autoincrement()),
            make_column("owner_id", &model::Bill::owner_id),
            make_column("event_id", &model::Bill::event_id),
            make_column("description", &model::Bill::description),
            make_column("amount", &model::Bill::amount),
            make_column("created_at", &model::Bill::created_at),
            make_column("has_annotation", &model::Bill::has_annotation, default_value(false)),
            foreign_key(&model::Bill::owner_id).references(&model::User::id),
            foreign_key(&model::Bill::event_id).references(&model::Event::id)
        ),

        make_table("annotations",
            make_column("id", &model::Annotation::id, primary_key().autoincrement()),
            make_column("bill_id", &model::Annotation::bill_id),
            make_column("content", &model::Annotation::content),
            make_column("authorid", &model::Annotation::authorid),
            make_column("created_at", &model::Annotation::created_at),
            foreign_key(&model::Annotation::bill_id).references(&model::Bill::id)
        )
    );
}

using Storage = decltype(CreateStorage(""));

class DatabaseORM {
public:
    explicit DatabaseORM(const std::string& db_path);
    
    Storage& GetStorage() { return storage_; }
    
    void Initialize();
    
private:
    Storage storage_;
    std::string db_path_;
};