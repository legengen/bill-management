#include "DatabaseTestBase.h"

class EventRepositoryTest : public DatabaseTestBase {};

// ==================== save 测试 ====================

TEST_F(EventRepositoryTest, Save_NewEvent_Success) {
    // Arrange
    auto event = CreateEvent("娱乐");
    
    // Act
    event_repo_->save(event);
    
    // Assert
    auto saved = event_repo_->findByName("娱乐");
    ASSERT_TRUE(saved.has_value());
    EXPECT_EQ(saved->name, "娱乐");
    EXPECT_EQ(saved->status, model::EventStatus::Available);
}

TEST_F(EventRepositoryTest, Save_UpdateExisting_Success) {
    // Arrange
    auto event = event_repo_->findByName("餐饮");
    ASSERT_TRUE(event.has_value());
    
    // Act
    event->status = model::EventStatus::Frozen;
    event_repo_->save(*event);
    
    // Assert
    auto updated = event_repo_->findByName("餐饮");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, model::EventStatus::Frozen);
}

// ==================== findById 测试 ====================

TEST_F(EventRepositoryTest, FindById_Exists_ReturnsEvent) {
    // Arrange
    auto event = event_repo_->findByName("餐饮");
    ASSERT_TRUE(event.has_value());
    int event_id = event->id;
    
    // Act
    auto found = event_repo_->findById(event_id);
    
    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "餐饮");
}

TEST_F(EventRepositoryTest, FindById_NotExists_ReturnsNullopt) {
    // Act
    auto found = event_repo_->findById(99999);
    
    // Assert
    EXPECT_FALSE(found.has_value());
}

// ==================== findByName 测试 ====================

TEST_F(EventRepositoryTest, FindByName_Exists_ReturnsEvent) {
    // Act
    auto found = event_repo_->findByName("交通");
    
    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "交通");
}

TEST_F(EventRepositoryTest, FindByName_NotExists_ReturnsNullopt) {
    // Act
    auto found = event_repo_->findByName("不存在的事件");
    
    // Assert
    EXPECT_FALSE(found.has_value());
}

TEST_F(EventRepositoryTest, FindByName_CaseSensitive) {
    // Act
    auto lower = event_repo_->findByName("餐饮");
    
    // Assert
    ASSERT_TRUE(lower.has_value());
}

// ==================== setStatusById 测试 ====================

TEST_F(EventRepositoryTest, SetStatusById_ToFrozen_Success) {
    // Arrange
    auto event = event_repo_->findByName("餐饮");
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->status, model::EventStatus::Available);
    
    // Act
    bool result = event_repo_->setStatusById(event->id, model::EventStatus::Frozen);
    
    // Assert
    EXPECT_TRUE(result);
    
    auto updated = event_repo_->findById(event->id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, model::EventStatus::Frozen);
}

TEST_F(EventRepositoryTest, SetStatusById_ToAvailable_Success) {
    // Arrange
    auto event = event_repo_->findByName("购物");
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->status, model::EventStatus::Frozen);
    
    // Act
    bool result = event_repo_->setStatusById(event->id, model::EventStatus::Available);
    
    // Assert
    EXPECT_TRUE(result);
    
    auto updated = event_repo_->findById(event->id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, model::EventStatus::Available);
}

TEST_F(EventRepositoryTest, SetStatusById_NotExists_ReturnsFalse) {
    // Act
    bool result = event_repo_->setStatusById(99999, model::EventStatus::Frozen);
    
    // Assert
    EXPECT_FALSE(result);
}