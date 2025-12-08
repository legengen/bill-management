#include "DatabaseTestBase.h"

class BillRepositoryTest : public DatabaseTestBase {
protected:
    void SetUp() override {
        DatabaseTestBase::SetUp();
        
        // 创建测试账单
        auto user = user_repo_->queryByPhone("13800000001");
        auto event = event_repo_->findByName("餐饮");
        
        if (user.has_value() && event.has_value()) {
            for (int i = 0; i < 5; ++i) {
                model::Bill bill;
                bill.owner_id = user->id;
                bill.event_id = event->id;
                bill.amount = 100.0 * (i + 1);
                bill.description = "TestBill_" + std::to_string(i);
                bill.created_at = model::Now() - (5 - i) * 3600;  // 每小时一条
                bill_repo_->save(bill);
            }
        }
    }
};

// ==================== save 测试 ====================

// 暂时禁用，目前save无法返回id，无法通过id获取save信息
TEST_F(BillRepositoryTest, DISABLED_Save_NewBill_Success) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    auto event = event_repo_->findByName("交通");
    ASSERT_TRUE(user.has_value());
    ASSERT_TRUE(event.has_value());
    
    auto bill = CreateBill(user->id, event->id, 88.50, "Bus ticket");
    
    // Act
    bill_repo_->save(bill);
    
    // Assert
    auto saved = bill_repo_->findById(bill.id);
    ASSERT_TRUE(saved.has_value());
    EXPECT_DOUBLE_EQ(saved->amount, 88.50);
    EXPECT_EQ(saved->description, "Bus ticket");
}

TEST_F(BillRepositoryTest, Save_UpdateExisting_Success) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    auto event = event_repo_->findByName("餐饮");
    
    auto bills = bill_repo_->queryByEvent(user->id, event->id);
    ASSERT_FALSE(bills.empty());
    
    auto bill = bills[0];
    double original_amount = bill.amount;
    
    // Act
    bill.amount = 999.99;
    bill.description = "Updated description";
    bill_repo_->save(bill);
    
    // Assert
    auto updated = bill_repo_->findById(bill.id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_DOUBLE_EQ(updated->amount, 999.99);
    EXPECT_EQ(updated->description, "Updated description");
}

// ==================== findById 测试 ====================

TEST_F(BillRepositoryTest, FindById_Exists_ReturnsBill) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    auto event = event_repo_->findByName("餐饮");
    auto bills = bill_repo_->queryByEvent(user->id, event->id);
    ASSERT_FALSE(bills.empty());
    
    int bill_id = bills[0]. id;
    
    // Act
    auto found = bill_repo_->findById(bill_id);
    
    // Assert
    ASSERT_TRUE(found. has_value());
    EXPECT_EQ(found->id, bill_id);
}

TEST_F(BillRepositoryTest, FindById_NotExists_ReturnsNullopt) {
    // Act
    auto found = bill_repo_->findById(99999);
    
    // Assert
    EXPECT_FALSE(found.has_value());
}

// ==================== queryByEvent 测试 ====================

TEST_F(BillRepositoryTest, QueryByEvent_HasBills_ReturnsList) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    auto event = event_repo_->findByName("餐饮");
    
    // Act
    auto bills = bill_repo_->queryByEvent(user->id, event->id);
    
    // Assert
    EXPECT_EQ(bills.size(), 5);
}

TEST_F(BillRepositoryTest, QueryByEvent_NoBills_ReturnsEmpty) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    auto event = event_repo_->findByName("购物");
    
    // Act
    auto bills = bill_repo_->queryByEvent(user->id, event->id);
    
    // Assert
    EXPECT_TRUE(bills.empty());
}

// ==================== queryByTime 测试 ====================

TEST_F(BillRepositoryTest, QueryByTime_WithinRange_ReturnsBills) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    model::Timestamp from = model::Now() - 24 * 3600;  // 24小时前
    model::Timestamp to = model::Now();
    
    // Act
    auto bills = bill_repo_->queryByTime(user->id, from, to);
    
    // Assert
    EXPECT_EQ(bills.size(), 5);
}

TEST_F(BillRepositoryTest, QueryByTime_OutsideRange_ReturnsEmpty) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    model::Timestamp from = model::Now() - 48 * 3600;  // 48小时前
    model::Timestamp to = model::Now() - 24 * 3600;    // 24小时前
    
    // Act
    auto bills = bill_repo_->queryByTime(user->id, from, to);
    
    // Assert
    EXPECT_TRUE(bills. empty());
}

// ==================== queryByTimeInOrder 测试 ====================

TEST_F(BillRepositoryTest, QueryByTimeInOrder_ReturnsOrderedByTime) {
    // Arrange
    model::Timestamp from = model::Now() - 24 * 3600;
    model::Timestamp to = model::Now();
    
    // Act
    auto bills = bill_repo_->queryByTimeInOrder(from, to);
    
    // Assert
    ASSERT_EQ(bills.size(), 5);
    
    // 验证时间升序
    for (size_t i = 1; i < bills.size(); ++i) {
        EXPECT_LE(bills[i-1]. created_at, bills[i]. created_at);
    }
}

// ==================== remove 测试 ====================

TEST_F(BillRepositoryTest, Remove_Exists_DeletesBill) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    auto event = event_repo_->findByName("餐饮");
    auto bills = bill_repo_->queryByEvent(user->id, event->id);
    ASSERT_FALSE(bills.empty());
    
    int bill_id = bills[0]. id;
    size_t original_count = bills.size();
    
    // Act
    bill_repo_->remove(bill_id);
    
    // Assert
    auto deleted = bill_repo_->findById(bill_id);
    EXPECT_FALSE(deleted.has_value());
    
    auto remaining = bill_repo_->queryByEvent(user->id, event->id);
    EXPECT_EQ(remaining.size(), original_count - 1);
}

TEST_F(BillRepositoryTest, Remove_NotExists_NoError) {
    // Act & Assert - 不应该抛出异常
    EXPECT_NO_THROW(bill_repo_->remove(99999));
}