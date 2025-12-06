#include "AnnotationRepositoryImpl.h"

using namespace sqlite_orm;

void AnnotationRepositoryImpl::save(const model::Annotation& a) {
    auto& storage = db_->GetStorage();
    
    if (a.id == 0) {
        // 插入新注解
        model::Annotation annotation_copy = a;
        annotation_copy.id = storage.insert(annotation_copy);
    } else {
        // 更新现有注解
        storage. update(a);
    }
}

std::optional<model::Annotation> AnnotationRepositoryImpl::findById(int id) {
    auto& storage = db_->GetStorage();
    
    try {
        auto annotation = storage.get<model::Annotation>(id);
        return annotation;
    } catch (const std::system_error&) {
        // 记录未找到
        return std::nullopt;
    } catch (const std::exception& ex) {
        // 其他错误
        return std::nullopt;
    }
}

// 额外的辅助方法实现

std::vector<model::Annotation> AnnotationRepositoryImpl::findByBillId(int bill_id) {
    auto& storage = db_->GetStorage();
    
    try {
        return storage.get_all<model::Annotation>(
            where(c(&model::Annotation::bill_id) == bill_id),
            order_by(&model::Annotation::created_at). desc()
        );
    } catch (const std::exception& ex) {
        return {};
    }
}

std::vector<model::Annotation> AnnotationRepositoryImpl::findByAuthorId(int author_id) {
    auto& storage = db_->GetStorage();
    
    try {
        return storage.get_all<model::Annotation>(
            where(c(&model::Annotation::authorid) == author_id),
            order_by(&model::Annotation::created_at).desc()
        );
    } catch (const std::exception& ex) {
        return {};
    }
}

void AnnotationRepositoryImpl::removeByBillId(int bill_id) {
    auto& storage = db_->GetStorage();
    
    try {
        storage.remove_all<model::Annotation>(
            where(c(&model::Annotation::bill_id) == bill_id)
        );
    } catch (const std::exception& ex) {
        // 忽略错误或记录日志
    }
}