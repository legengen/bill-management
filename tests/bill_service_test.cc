#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <BillService.h>
#include <irepositories.h>
#include <models.h>

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::Invoke;

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

class BillServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_repo_ = std::make_shared<NiceMock<MockBillRepository>>();
        bill_service_ = std::make_unique<BillService>(mock_repo_);
        
        // 设置基准时间
        base_time_ = std::chrono::system_clock::now();
    }

    void TearDown() override {
        bill_service_.reset();
        mock_repo_.reset();
    }

    // 辅助函数：创建测试账单
    model::Bill CreateTestBill(int id = 1,
                               int owner_id = 1,
                               double amount = 100.0,
                               const std::string& description = "Test Bill") {
        model::Bill bill;
        bill.id = id;
        bill.owner_id = owner_id;
        bill. amount = amount;
        bill. description = description;
        bill. created_at = base_time_;
        
        // 创建默认事件
        bill.event.id = 1;
        bill.event.name = "Test Event";
        bill.event. status = model::EventStatus::Available;
        
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
        return annotation;
    }

    std::shared_ptr<MockBillRepository> mock_repo_;
    std::unique_ptr<BillService> bill_service_;
    model::Timestamp base_time_;
};

// ==================== CreateBill Tests ====================

TEST_F(BillServiceTest, CreateBill_Success_ValidData) {
    // Arrange
    const int owner_id = 1;
    model::Bill new_bill = CreateTestBill(0, owner_id, 150.50, "Lunch");
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    auto result = bill_service_->CreateBill(owner_id, new_bill);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->amount, 150.50);
    EXPECT_EQ(result->description, "Lunch");
    EXPECT_EQ(saved_bill.amount, 150.50);
}

TEST_F(BillServiceTest, CreateBill_Success_SetsIssueDate) {
    // Arrange
    const int owner_id = 1;
    model::Bill new_bill = CreateTestBill(1, owner_id, 100.0);
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    auto before_time = std::chrono::system_clock::now();
    new_bill.created_at = std::chrono::system_clock::now();
    
    // Act
    auto result = bill_service_->CreateBill(owner_id, new_bill);
    
    auto after_time = std::chrono::system_clock::now();
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->created_at > before_time);
    EXPECT_TRUE(result->created_at < after_time);
}

TEST_F(BillServiceTest, CreateBill_Failure_InvalidOwnerId_Zero) {
    // Arrange
    const int invalid_owner_id = 0;
    model::Bill new_bill = CreateTestBill(0, 1, 100.0);
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    auto result = bill_service_->CreateBill(invalid_owner_id, new_bill);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(BillServiceTest, CreateBill_Failure_InvalidOwnerId_Negative) {
    // Arrange
    const int invalid_owner_id = -1;
    model::Bill new_bill = CreateTestBill(0, 1, 100.0);
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    auto result = bill_service_->CreateBill(invalid_owner_id, new_bill);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(BillServiceTest, CreateBill_Failure_ZeroAmount) {
    // Arrange
    const int owner_id = 1;
    model::Bill new_bill = CreateTestBill(0, owner_id, 0.0);
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    auto result = bill_service_->CreateBill(owner_id, new_bill);
    
    // Assert
    EXPECT_FALSE(result. has_value());
}

TEST_F(BillServiceTest, CreateBill_Failure_NegativeAmount) {
    // Arrange
    const int owner_id = 1;
    model::Bill new_bill = CreateTestBill(0, owner_id, -50.0);
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    auto result = bill_service_->CreateBill(owner_id, new_bill);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(BillServiceTest, CreateBill_Success_WithEvent) {
    // Arrange
    const int owner_id = 1;
    model::Bill new_bill = CreateTestBill(0, owner_id, 200.0, "Dinner");
    new_bill.event.id = 5;
    new_bill.event.name = "Restaurant";
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    auto result = bill_service_->CreateBill(owner_id, new_bill);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->event.id, 5);
    EXPECT_EQ(result->event.name, "Restaurant");
}

// ==================== QueryByTime Tests ====================

TEST_F(BillServiceTest, QueryByTime_Success_MultipleResults) {
    // Arrange
    const int owner_id = 1;
    auto from_time = base_time_ - std::chrono::hours(24);
    auto to_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, owner_id, 100.0, "Bill 1"),
        CreateTestBill(2, owner_id, 200.0, "Bill 2"),
        CreateTestBill(3, owner_id, 300.0, "Bill 3")
    };
    
    EXPECT_CALL(*mock_repo_, queryByTime(owner_id, from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = bill_service_->QueryByTime(owner_id, from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0]. description, "Bill 1");
    EXPECT_EQ(results[1].description, "Bill 2");
    EXPECT_EQ(results[2].description, "Bill 3");
}

TEST_F(BillServiceTest, QueryByTime_Success_NoResults) {
    // Arrange
    const int owner_id = 1;
    auto from_time = base_time_ - std::chrono::hours(48);
    auto to_time = base_time_ - std::chrono::hours(24);
    
    std::vector<model::Bill> empty_bills;
    
    EXPECT_CALL(*mock_repo_, queryByTime(owner_id, from_time, to_time))
        .WillOnce(Return(empty_bills));
    
    // Act
    auto results = bill_service_->QueryByTime(owner_id, from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(BillServiceTest, QueryByTime_Failure_InvalidOwnerId) {
    // Arrange
    const int invalid_owner_id = 0;
    auto from_time = base_time_ - std::chrono::hours(24);
    auto to_time = base_time_;
    
    EXPECT_CALL(*mock_repo_, queryByTime(_, _, _))
        .Times(0);
    
    // Act
    auto results = bill_service_->QueryByTime(invalid_owner_id, from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(BillServiceTest, QueryByTime_Failure_InvalidTimeRange) {
    // Arrange
    const int owner_id = 1;
    auto from_time = base_time_;
    auto to_time = base_time_ - std::chrono::hours(24);  // to_time < from_time
    
    EXPECT_CALL(*mock_repo_, queryByTime(_, _, _))
        .Times(0);
    
    // Act
    auto results = bill_service_->QueryByTime(owner_id, from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(BillServiceTest, QueryByTime_Success_SameTime) {
    // Arrange
    const int owner_id = 1;
    auto same_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, owner_id, 100.0)
    };
    
    EXPECT_CALL(*mock_repo_, queryByTime(owner_id, same_time, same_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = bill_service_->QueryByTime(owner_id, same_time, same_time);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
}

// ==================== queryByEvent Tests ====================

TEST_F(BillServiceTest, QueryByEvent_Success_MultipleResults) {
    // Arrange
    const int owner_id = 1;
    const int event_id = 5;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, owner_id, 100.0, "Bill 1"),
        CreateTestBill(2, owner_id, 200.0, "Bill 2")
    };
    expected_bills[0].event.id = event_id;
    expected_bills[1].event.id = event_id;
    
    EXPECT_CALL(*mock_repo_, queryByEvent(owner_id, event_id))
        . WillOnce(Return(expected_bills));
    
    // Act
    auto results = bill_service_->queryByEvent(owner_id, event_id);
    
    // Assert
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].event.id, event_id);
    EXPECT_EQ(results[1].event.id, event_id);
}

TEST_F(BillServiceTest, QueryByEvent_Success_NoResults) {
    // Arrange
    const int owner_id = 1;
    const int event_id = 999;
    
    std::vector<model::Bill> empty_bills;
    
    EXPECT_CALL(*mock_repo_, queryByEvent(owner_id, event_id))
        . WillOnce(Return(empty_bills));
    
    // Act
    auto results = bill_service_->queryByEvent(owner_id, event_id);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(BillServiceTest, QueryByEvent_Failure_InvalidOwnerId) {
    // Arrange
    const int invalid_owner_id = 0;
    const int event_id = 5;
    
    EXPECT_CALL(*mock_repo_, queryByEvent(_, _))
        .Times(0);
    
    // Act
    auto results = bill_service_->queryByEvent(invalid_owner_id, event_id);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(BillServiceTest, QueryByEvent_Failure_InvalidEventId) {
    // Arrange
    const int owner_id = 1;
    const int invalid_event_id = 0;
    
    EXPECT_CALL(*mock_repo_, queryByEvent(_, _))
        .Times(0);
    
    // Act
    auto results = bill_service_->queryByEvent(owner_id, invalid_event_id);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(BillServiceTest, QueryByEvent_Failure_BothInvalidIds) {
    // Arrange
    const int invalid_owner_id = -1;
    const int invalid_event_id = -1;
    
    EXPECT_CALL(*mock_repo_, queryByEvent(_, _))
        .Times(0);
    
    // Act
    auto results = bill_service_->queryByEvent(invalid_owner_id, invalid_event_id);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

// ==================== queryByPhone Tests ====================

TEST_F(BillServiceTest, QueryByPhone_Success_MultipleResults) {
    // Arrange
    const std::string phone = "13800138000";
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 100.0, "Bill 1"),
        CreateTestBill(2, 1, 200.0, "Bill 2")
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        . WillOnce(Return(expected_bills));
    
    // Act
    auto results = bill_service_->queryByPhone(phone);
    
    // Assert
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].description, "Bill 1");
    EXPECT_EQ(results[1].description, "Bill 2");
}

TEST_F(BillServiceTest, QueryByPhone_Success_NoResults) {
    // Arrange
    const std::string phone = "99999999999";
    
    std::vector<model::Bill> empty_bills;
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(empty_bills));
    
    // Act
    auto results = bill_service_->queryByPhone(phone);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(BillServiceTest, QueryByPhone_Failure_EmptyPhone) {
    // Arrange
    const std::string empty_phone = "";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(_))
        .Times(0);
    
    // Act
    auto results = bill_service_->queryByPhone(empty_phone);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

// ==================== editBill Tests ====================

TEST_F(BillServiceTest, EditBill_Success_UpdateDescription) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0, "Old Description");
    model::Bill updates = CreateTestBill(0, 1, 100.0, "New Description");
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->editBill(bill_id, updates);
    
    // Assert
    EXPECT_EQ(saved_bill. id, bill_id);
    EXPECT_EQ(saved_bill.description, "New Description");
}

TEST_F(BillServiceTest, EditBill_Success_UpdateAmount) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0);
    model::Bill updates = CreateTestBill(0, 1, 250.75);
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        . WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        . Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->editBill(bill_id, updates);
    
    // Assert
    EXPECT_EQ(saved_bill. id, bill_id);
    EXPECT_DOUBLE_EQ(saved_bill. amount, 250.75);
}

TEST_F(BillServiceTest, EditBill_Success_PreservesBillId) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0);
    model::Bill updates = CreateTestBill(999, 1, 200.0);  // 不同的 ID
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        . WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->editBill(bill_id, updates);
    
    // Assert
    EXPECT_EQ(saved_bill.id, bill_id);  // 应该保留原有 ID
}

TEST_F(BillServiceTest, EditBill_Failure_BillNotFound) {
    // Arrange
    const int bill_id = 999;
    model::Bill updates = CreateTestBill(0, 1, 200.0);
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->editBill(bill_id, updates);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceTest, EditBill_Failure_InvalidBillId) {
    // Arrange
    const int invalid_bill_id = 0;
    model::Bill updates = CreateTestBill(0, 1, 200.0);
    
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->editBill(invalid_bill_id, updates);
    
    // Assert - 无异常即通过
}

// ==================== deleteBill Tests ====================

TEST_F(BillServiceTest, DeleteBill_Success) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0);
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    EXPECT_CALL(*mock_repo_, remove(bill_id))
        .Times(1);
    
    // Act
    bill_service_->deleteBill(bill_id);
    
    // Assert - 验证调用
}

TEST_F(BillServiceTest, DeleteBill_Failure_BillNotFound) {
    // Arrange
    const int bill_id = 999;
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, remove(_))
        .Times(0);
    
    // Act
    bill_service_->deleteBill(bill_id);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceTest, DeleteBill_Failure_InvalidBillId_Zero) {
    // Arrange
    const int invalid_bill_id = 0;
    
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, remove(_))
        .Times(0);
    
    // Act
    bill_service_->deleteBill(invalid_bill_id);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceTest, DeleteBill_Failure_InvalidBillId_Negative) {
    // Arrange
    const int invalid_bill_id = -1;
    
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, remove(_))
        .Times(0);
    
    // Act
    bill_service_->deleteBill(invalid_bill_id);
    
    // Assert - 无异常即通过
}

// ==================== annotateBill Tests ====================

TEST_F(BillServiceTest, AnnotateBill_Success) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0);
    model::Annotation annotation = CreateTestAnnotation(1, "Important note", 2);
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        . WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation. id, 1);
    EXPECT_EQ(saved_bill.annotation. content, "Important note");
    EXPECT_EQ(saved_bill.annotation.authorid, 2);
}

TEST_F(BillServiceTest, AnnotateBill_Success_UpdateExistingAnnotation) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0);
    existing_bill.annotation = CreateTestAnnotation(1, "Old note", 1);
    
    model::Annotation new_annotation = CreateTestAnnotation(2, "New note", 2);
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        . WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        . Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, new_annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation.content, "New note");
    EXPECT_EQ(saved_bill.annotation.authorid, 2);
}

TEST_F(BillServiceTest, AnnotateBill_Failure_BillNotFound) {
    // Arrange
    const int bill_id = 999;
    model::Annotation annotation = CreateTestAnnotation(1, "Note", 1);
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->annotateBill(bill_id, annotation);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceTest, AnnotateBill_Failure_InvalidBillId) {
    // Arrange
    const int invalid_bill_id = 0;
    model::Annotation annotation = CreateTestAnnotation(1, "Note", 1);
    
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    bill_service_->annotateBill(invalid_bill_id, annotation);
    
    // Assert - 无异常即通过
}

TEST_F(BillServiceTest, AnnotateBill_Success_EmptyContent) {
    // Arrange
    const int bill_id = 1;
    model::Bill existing_bill = CreateTestBill(bill_id, 1, 100.0);
    model::Annotation empty_annotation = CreateTestAnnotation(1, "", 1);
    
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .WillOnce(Return(existing_bill));
    
    model::Bill saved_bill;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_bill));
    
    // Act
    bill_service_->annotateBill(bill_id, empty_annotation);
    
    // Assert
    EXPECT_EQ(saved_bill.annotation. content, "");
}

// ==================== Integration Tests ====================

TEST_F(BillServiceTest, Integration_CreateAndQueryByTime) {
    // Arrange
    const int owner_id = 1;
    model::Bill new_bill = CreateTestBill(0, owner_id, 100.0);
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    auto from_time = base_time_ - std::chrono::hours(1);
    auto to_time = base_time_ + std::chrono::hours(1);
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, owner_id, 100.0)
    };
    
    EXPECT_CALL(*mock_repo_, queryByTime(owner_id, from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto create_result = bill_service_->CreateBill(owner_id, new_bill);
    auto query_results = bill_service_->QueryByTime(owner_id, from_time, to_time);
    
    // Assert
    ASSERT_TRUE(create_result.has_value());
    ASSERT_EQ(query_results.size(), 1);
}

TEST_F(BillServiceTest, Integration_CreateEditDelete) {
    // Arrange
    const int owner_id = 1;
    const int bill_id = 1;
    
    // 创建
    model::Bill new_bill = CreateTestBill(0, owner_id, 100.0, "Original");
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(2);  // Create + Edit
    
    // 编辑
    model::Bill existing_bill = CreateTestBill(bill_id, owner_id, 100.0, "Original");
    model::Bill updates = CreateTestBill(0, owner_id, 200.0, "Updated");
    EXPECT_CALL(*mock_repo_, findById(bill_id))
        .Times(2)  // Edit + Delete
        .WillRepeatedly(Return(existing_bill));
    
    // 删除
    EXPECT_CALL(*mock_repo_, remove(bill_id))
        .Times(1);
    
    // Act
    auto create_result = bill_service_->CreateBill(owner_id, new_bill);
    bill_service_->editBill(bill_id, updates);
    bill_service_->deleteBill(bill_id);
    
    // Assert
    ASSERT_TRUE(create_result.has_value());
}