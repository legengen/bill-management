#include "DatabaseORM.h"
#include <iostream>

DatabaseORM::DatabaseORM(const std::string& db_path) 
    : storage_(CreateStorage(db_path)), db_path_(db_path) {
    Initialize();
}

void DatabaseORM::Initialize() {
    storage_.sync_schema();
}