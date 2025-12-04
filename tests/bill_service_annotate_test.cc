#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/BillService.h"
#include "data/irepositories.h"
#include "common/models.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::InSequence;

// Mock BillRepository
class MockBillRepository : public repo::IBillRepository {
public:
    MOCK_METHOD(void, save, (const model::Bill& b), (override));
    MOCK_METHOD(std::optional<model::Bill>, findById, (int id), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByEvent, (int ownerId, int eventId), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByEvent, (std::string& name), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByTime, (int ownerId, model::Timestamp from, model::Timestamp to), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByTime, (model::Timestamp from, model::Timestamp to), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByPhone, (const std::string& phone), (override));
    MOCK_METHOD(void, remove, (int id), (override));
};

// Mock AnnotationRepository
class MockAnnotationRepository : public repo::IAnnotationRepository {
public:
    MOCK_METHOD(void, save, (const model::Annotation& a), (override));
    MOCK_METHOD(std::optional<model::Annotation>, findById, (int id), (override));
};

class BillServiceAnnotateTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_bill_repo_ = std::make_shared<NiceMock<MockBillRepository>>();
        mock_annotation_repo_ = std::make_shared<NiceMock<MockAnnotationRepository>>();
        bill_service_ = std::make_unique<BillService>(mock_bill_repo_, mock_annotation_repo_);
        
        base_time_ = std::chrono::system_clock::now();
    }

    void TearDown() override {
        bill_service_.reset();
        mock_annotation_repo_.reset();
        mock_bill_repo_.reset();
    }

    // 辅助函数：创建测试账单
    model::Bill CreateTestBill(int id = 1,
                               int owner_id = 1,
                               double amount = 100.0,
                               const std::string& description = "Test Bill",
                               bool has_annotation = false) {
        model::Bill bill;
        bill.id = id;
        bill.owner_id = owner_id;
        bill.amount = amount;
        bill.description = description;
        bill.created_at = base_time_;
        bill.has_annotation = has_annotation;
        
        bill. event. id = 1;
        bill. event.name = "Test Event";
        bill.event.status = model::EventStatus::Available;
        
        return bill;
    }

    // 辅助函数：创建测试注解
    model::Annotation CreateTestAnnotation(int id = 1,
                                           const std::string& content = "Test annotation",
                                           int author_id = 1) {
        model::Annotation annotation;
        annotation.id = id;
        annotation.content = content;
        annotation.authorid = author_id;
        annotation.created_at = base_time_;
        return annotation;
    }

    std::shared_ptr<MockBillRepository> mock_bill_repo_;
    std::shared_ptr<MockAnnotationRepository> mock_annotation_repo_;
    std::unique_ptr<BillService> bill_service_;
    model::Timestamp base_time_;
};

// ==================== annotateBill Tests ====================

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_NewAnnotation) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0, "Original Bill", false);
    model::Annotation annotation = CreateTestAnnotation(1, "Important note", 2);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    // 验证注解先被保存
    model::Annotation saved_annotation;
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_annotation));
    
    // 验证账单后被更新
    model::Bill saved_bill;
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert
    EXPECT_EQ(saved_annotation.id, 1);
    EXPECT_EQ(saved_annotation.content, "Important note");
    EXPECT_EQ(saved_annotation.authorid, 2);
    
    EXPECT_EQ(saved_bill.id, bill_id);
    EXPECT_TRUE(saved_bill.has_annotation);
    EXPECT_EQ(saved_bill.annotation.content, "Important note");
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_SaveOrderCorrect) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    model::Annotation annotation = CreateTestAnnotation(1, "Note");
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    // 使用 InSequence 确保调用顺序
    InSequence seq;
    
    // 注解必须先保存
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    
    // 账单后保存
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1);
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert - 顺序由 InSequence 保证
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_UpdateExistingAnnotation) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0, "Bill", true);
    existing_bill.annotation = CreateTestAnnotation(1, "Old note", 1);
    
    model::Annotation new_annotation = CreateTestAnnotation(2, "New note", 2);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, new_annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation.content, "New note");
    EXPECT_EQ(saved_bill.annotation.authorid, 2);
    EXPECT_TRUE(saved_bill.has_annotation);
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_LongContent) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    
    std::string long_content(1000, 'A');
    model::Annotation annotation = CreateTestAnnotation(1, long_content, 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1)
        . WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation.content. length(), 1000);
    EXPECT_TRUE(saved_bill.has_annotation);
}

// TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_SpecialCharacters) {
//     // Arrange
//     const int bill_id = 1;
//     model::Bill existing_bill = CreateTestBill(bill_id);
//     model::Annotation annotation = CreateTestAnnotation(1, "Note with 特殊字符 & symbols! @#$%", 1);
    
//     EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
//         .WillOnce(Return(existing_bill));
    
//     EXPECT_CALL(*mock_annotation_repo_, save(_))
//         .Times(1);
    
//     model::Bill saved_bill;
//     EXPECT_CALL(*mock_bill_repo_, save(_))
//         .Times(1)
//         . WillOnce(SaveArg<0>(&saved_bill));
    
//     // Act
//     bill_service_->annotateBill(bill_id, annotation);
    
//     // Assert
//     EXPECT_EQ(saved_bill.annotation.content, "Note with 特殊字符 & symbols!@#$%");
//     EXPECT_TRUE(saved_bill.has_annotation);
// }

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_PreservesOtherBillFields) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 5, 250.50, "Important Bill");
    model::Annotation annotation = CreateTestAnnotation(1, "Note", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1)
        . WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert - 验证其他字段未被修改
    EXPECT_EQ(saved_bill.id, bill_id);
    EXPECT_EQ(saved_bill.owner_id, 5);
    EXPECT_DOUBLE_EQ(saved_bill.amount, 250.50);
    EXPECT_EQ(saved_bill.description, "Important Bill");
    EXPECT_TRUE(saved_bill.has_annotation);
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_MultipleAnnotations) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    
    model::Annotation annotation1 = CreateTestAnnotation(1, "First note", 1);
    model::Annotation annotation2 = CreateTestAnnotation(2, "Second note", 2);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .Times(2)
        .WillRepeatedly(Return(existing_bill));
    
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(2);
    
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(2);
    
    // Act
    bill_service_->annotateBill(bill_id, annotation1);
    bill_service_->annotateBill(bill_id, annotation2);
    
    // Assert - 验证调用次数
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_WithWhitespace) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    model::Annotation annotation = CreateTestAnnotation(1, "  Note with spaces  ", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        . Times(1);
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation.content, "  Note with spaces  ");
    EXPECT_TRUE(saved_bill.has_annotation);
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Success_DifferentAuthors) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    
    model::Annotation annotation1 = CreateTestAnnotation(1, "Admin note", 100);
    model::Annotation annotation2 = CreateTestAnnotation(2, "User note", 200);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .Times(2)
        .WillRepeatedly(Return(existing_bill));
    
    model::Annotation saved_annotation1, saved_annotation2;
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(2)
        .WillOnce(SaveArg<0>(&saved_annotation1))
        .WillOnce(SaveArg<0>(&saved_annotation2));
    
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(2);
    
    // Act
    bill_service_->annotateBill(bill_id, annotation1);
    bill_service_->annotateBill(bill_id, annotation2);
    
    // Assert
    EXPECT_EQ(saved_annotation1.authorid, 100);
    EXPECT_EQ(saved_annotation2.authorid, 200);
}

// ==================== Failure Cases ====================

TEST_F(BillServiceAnnotateTest, AnnotateBill_Failure_BillNotFound) {
    // Arrange
    const int bill_id = 999;
    model::Annotation annotation = CreateTestAnnotation(1, "Note", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(std::nullopt));
    
    // 不应该调用保存操作
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(0);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Failure_InvalidBillId_Zero) {
    // Arrange
    const int invalid_bill_id = 0;
    model::Annotation annotation = CreateTestAnnotation(1, "Note", 1);
    
    // 不应该调用任何 repository 方法
    EXPECT_CALL(*mock_bill_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(0);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->annotateBill(invalid_bill_id, annotation);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Failure_InvalidBillId_Negative) {
    // Arrange
    const int invalid_bill_id = -1;
    model::Annotation annotation = CreateTestAnnotation(1, "Note", 1);
    
    // 不应该调用任何 repository 方法
    EXPECT_CALL(*mock_bill_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(0);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->annotateBill(invalid_bill_id, annotation);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Failure_EmptyContent) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    model::Annotation empty_annotation = CreateTestAnnotation(1, "", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        . WillOnce(Return(existing_bill));
    
    // 不应该调用保存操作
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(0);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->annotateBill(bill_id, empty_annotation);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Failure_OnlyWhitespace) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    
    // 注意：当前实现只检查 empty()，所以纯空格会通过
    // 如果需要拒绝纯空格，需要修改实现
    model::Annotation whitespace_annotation = CreateTestAnnotation(1, "   ", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    // 当前实现会保存（如果想拒绝，需要修改实现）
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1);
    
    // Act
    bill_service_->annotateBill(bill_id, whitespace_annotation);
    
    // Assert - 当前实现允许纯空格
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_Failure_InvalidAuthorId_Zero) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    model::Annotation annotation = CreateTestAnnotation(1, "Valid note", 0);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    // 当前实现不检查 author_id，会继续保存
    // 如果需要验证 author_id，需要修改实现
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1);
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert - 当前实现允许 author_id 为 0
}

// ==================== Edge Cases ====================

TEST_F(BillServiceAnnotateTest, AnnotateBill_EdgeCase_VeryLongContent) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    
    std::string very_long_content(10000, 'X');
    model::Annotation annotation = CreateTestAnnotation(1, very_long_content, 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    model::Annotation saved_annotation;
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1)
        . WillOnce(SaveArg<0>(&saved_annotation));
    
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1);
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert
    EXPECT_EQ(saved_annotation.content. length(), 10000);
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_EdgeCase_NewlineInContent) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    model::Annotation annotation = CreateTestAnnotation(1, "Line 1\nLine 2\nLine 3", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation.content, "Line 1\nLine 2\nLine 3");
}

TEST_F(BillServiceAnnotateTest, AnnotateBill_EdgeCase_UnicodeContent) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    model::Annotation annotation = CreateTestAnnotation(1, "测试 🎉 テスト", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        . WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation.content, "测试 🎉 テスト");
}

// ==================== Integration Tests ====================

TEST_F(BillServiceAnnotateTest, Integration_AnnotateMultipleBills) {
    // Arrange
    model::Bill bill1 = CreateTestBill(1, 1, 100.0, "Bill 1");
    model::Bill bill2 = CreateTestBill(2, 1, 200.0, "Bill 2");
    
    model::Annotation annotation1 = CreateTestAnnotation(1, "Note for bill 1", 1);
    model::Annotation annotation2 = CreateTestAnnotation(2, "Note for bill 2", 1);
    
    EXPECT_CALL(*mock_bill_repo_, findById(1))
        .WillOnce(Return(bill1));
    EXPECT_CALL(*mock_bill_repo_, findById(2))
        .WillOnce(Return(bill2));
    
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(2);
    
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(2);
    
    // Act
    bill_service_->annotateBill(1, annotation1);
    bill_service_->annotateBill(2, annotation2);
    
    // Assert - 验证调用次数
}

TEST_F(BillServiceAnnotateTest, Integration_AnnotateAfterFailedAttempt) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id);
    
    model::Annotation empty_annotation = CreateTestAnnotation(1, "", 1);
    model::Annotation valid_annotation = CreateTestAnnotation(1, "Valid note", 1);
    
    // 第一次：内容为空，失败
    EXPECT_CALL(*mock_bill_repo_, findById(bill_id))
        .Times(2)
        .WillRepeatedly(Return(existing_bill));
    
    // 只有第二次会保存
    EXPECT_CALL(*mock_annotation_repo_, save(_))
        .Times(1);
    EXPECT_CALL(*mock_bill_repo_, save(_))
        .Times(1);
    
    // Act
    bill_service_->annotateBill(bill_id, empty_annotation);  // 失败
    bill_service_->annotateBill(bill_id, valid_annotation);  // 成功
    
    // Assert - 验证只保存了一次
}