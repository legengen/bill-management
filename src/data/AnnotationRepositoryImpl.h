#pragma once
#include "irepositories.h"
#include "DatabaseORM.h"
#include <memory>

class AnnotationRepositoryImpl : public repo::IAnnotationRepository {
public:
    explicit AnnotationRepositoryImpl(std::shared_ptr<DatabaseORM> db) : db_(db) {}
    
    void save(const model::Annotation& a) override;
    std::optional<model::Annotation> findById(int id) override;
    
    // 额外的辅助方法（可选）
    std::vector<model::Annotation> findByBillId(int bill_id);
    std::vector<model::Annotation> findByAuthorId(int author_id);
    void removeByBillId(int bill_id);
    
private:
    std::shared_ptr<DatabaseORM> db_;
};