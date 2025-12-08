#include "DatabaseTestBase.h"

class AnnotationRepositoryTest : public DatabaseTestBase {
protected:
    int test_bill_id_ = 0;
    int test_user_id_ = 0;
    
    void SetUp() override {
        DatabaseTestBase::SetUp();
        
        // 创建测试账单
        auto user = user_repo_->queryByPhone("13800000001");
        auto event = event_repo_->findByName("餐饮");
        
        if (user.has_value() && event.has_value()) {
            test_user_id_ = user->id;
            
            model::Bill bill;
            bill.owner_id = user->id;
            bill.event_id = event->id;
            bill.amount = 100.0;
            bill.description = "Test bill for annotation";
            bill_repo_->save(bill);
            
            test_bill_id_++;
        }
    }
};

// ==================== save 测试 ====================

TEST_F(AnnotationRepositoryTest, Save_NewAnnotation_Success) {
    // Arrange
    auto annotation = CreateAnnotation(test_bill_id_, test_user_id_, "This is a note");
    
    // Act
    annotation_repo_->save(annotation);
    annotation.id++;
    
    // Assert
    auto saved = annotation_repo_->findById(annotation.id);
    ASSERT_TRUE(saved.has_value());
    EXPECT_EQ(saved->content, "This is a note");
    EXPECT_EQ(saved->bill_id, test_bill_id_);
    EXPECT_EQ(saved->authorid, test_user_id_);
}

TEST_F(AnnotationRepositoryTest, Save_UpdateExisting_Success) {
    // Arrange
    auto annotation = CreateAnnotation(test_bill_id_, test_user_id_, "Original");
    annotation_repo_->save(annotation);
    annotation.id++;
    int annotation_id = annotation.id;
    
    // Act
    annotation.content = "Updated content";
    annotation_repo_->save(annotation);
    
    // Assert
    auto updated = annotation_repo_->findById(annotation_id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->content, "Updated content");
}

// ==================== findById 测试 ====================

TEST_F(AnnotationRepositoryTest, FindById_Exists_ReturnsAnnotation) {
    // Arrange
    auto annotation = CreateAnnotation(test_bill_id_, test_user_id_, "Test");
    annotation_repo_->save(annotation);
    annotation.id++;
    int annotation_id = annotation.id;
    
    // Act
    auto found = annotation_repo_->findById(annotation_id);
    
    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->content, "Test");
}

TEST_F(AnnotationRepositoryTest, FindById_NotExists_ReturnsNullopt) {
    // Act
    auto found = annotation_repo_->findById(99999);
    
    // Assert
    EXPECT_FALSE(found. has_value());
}

// ==================== 集成测试 ====================

TEST_F(AnnotationRepositoryTest, Integration_CreateAndUpdate) {
    // 1. 创建注解
    auto annotation = CreateAnnotation(test_bill_id_, test_user_id_, "Initial");
    annotation_repo_->save(annotation);
    annotation.id++;
    int annotation_id = annotation.id;
    
    // 2.  查询验证
    auto found = annotation_repo_->findById(annotation_id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->content, "Initial");
    
    // 3. 更新注解
    found->content = "Final version";
    annotation_repo_->save(*found);
    
    // 4. 再次查询验证
    auto updated = annotation_repo_->findById(annotation_id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->content, "Final version");
}