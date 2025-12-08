#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/EventService.h"
#include "data/irepositories.h"
#include "common/models.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::SaveArg;
using ::testing::Invoke;

// Mock EventRepository
class MockEventRepository : public repo::IEventRepository {
public:
    MOCK_METHOD(void, save, (const model::Event& e), (override));
    MOCK_METHOD(std::optional<model::Event>, findById, (int id), (override));
    MOCK_METHOD(std::optional<model::Event>, findByName, (const std::string& name), (override));
    MOCK_METHOD(bool, setStatusById, (int id, model::EventStatus status), (override));
};

class EventServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_repo_ = std::make_shared<NiceMock<MockEventRepository>>();
        event_service_ = std::make_unique<EventService>(mock_repo_);
        
        base_time_ = std::chrono::system_clock::now();
    }

    void TearDown() override {
        event_service_.reset();
        mock_repo_.reset();
    }

    // 辅助函数：创建测试事件
    model::Event CreateTestEvent(int id = 1,
                                  const std::string& name = "Test Event",
                                  model::EventStatus status = model::EventStatus::Available) {
        model::Event event;
        event.id = id;
        event.name = name;
        event.status = status;
        event.created_at = base_time_;
        return event;
    }

    std::shared_ptr<MockEventRepository> mock_repo_;
    std::unique_ptr<EventService> event_service_;
    model::Timestamp base_time_;
};

// ==================== QueryByName Tests ====================

TEST_F(EventServiceTest, QueryByName_Success_EventExists) {
    // Arrange
    const std::string event_name = "Shopping";
    model::Event expected_event = CreateTestEvent(1, event_name, model::EventStatus::Available);
    
    EXPECT_CALL(*mock_repo_, findByName(event_name))
        .WillOnce(Return(expected_event));
    
    // Act
    auto result = event_service_->QueryByName(event_name);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->name, event_name);
    EXPECT_EQ(result->status, model::EventStatus::Available);
}

TEST_F(EventServiceTest, QueryByName_Failure_EventNotFound) {
    // Arrange
    const std::string event_name = "NonExistent";
    
    EXPECT_CALL(*mock_repo_, findByName(event_name))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result = event_service_->QueryByName(event_name);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(EventServiceTest, QueryByName_Failure_EmptyName) {
    // Arrange
    const std::string empty_name = "";
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, findByName(_))
        .Times(0);
    
    // Act
    auto result = event_service_->QueryByName(empty_name);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(EventServiceTest, QueryByName_Success_FrozenEvent) {
    // Arrange
    const std::string event_name = "Frozen Event";
    model::Event expected_event = CreateTestEvent(2, event_name, model::EventStatus::Frozen);
    
    EXPECT_CALL(*mock_repo_, findByName(event_name))
        .WillOnce(Return(expected_event));
    
    // Act
    auto result = event_service_->QueryByName(event_name);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, model::EventStatus::Frozen);
}

TEST_F(EventServiceTest, QueryByName_Success_CaseSensitive) {
    // Arrange
    const std::string event_name = "Shopping";
    const std::string different_case = "shopping";
    
    EXPECT_CALL(*mock_repo_, findByName(event_name))
        .WillOnce(Return(CreateTestEvent(1, event_name)));
    
    EXPECT_CALL(*mock_repo_, findByName(different_case))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result1 = event_service_->QueryByName(event_name);
    auto result2 = event_service_->QueryByName(different_case);
    
    // Assert
    EXPECT_TRUE(result1.has_value());
    EXPECT_FALSE(result2.has_value());
}

TEST_F(EventServiceTest, QueryByName_Success_SpecialCharacters) {
    // Arrange
    const std::string event_name = "Event-2024_v1. 0";
    model::Event expected_event = CreateTestEvent(1, event_name);
    
    EXPECT_CALL(*mock_repo_, findByName(event_name))
        .WillOnce(Return(expected_event));
    
    // Act
    auto result = event_service_->QueryByName(event_name);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, event_name);
}

TEST_F(EventServiceTest, QueryByName_Success_LongName) {
    // Arrange
    const std::string long_name = std::string(100, 'A');
    model::Event expected_event = CreateTestEvent(1, long_name);
    
    EXPECT_CALL(*mock_repo_, findByName(long_name))
        .WillOnce(Return(expected_event));
    
    // Act
    auto result = event_service_->QueryByName(long_name);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, long_name);
}

// ==================== CreateEvent Tests ====================

TEST_F(EventServiceTest, CreateEvent_Success_NewEvent) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "New Event", model::EventStatus::Available);
    
    // 事件不存在
    EXPECT_CALL(*mock_repo_, findByName("New Event"))
        .WillOnce(Return(std::nullopt));
    
    model::Event saved_event;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_event));
    
    // Act
    auto result = event_service_->CreateEvent(new_event);
    
    // Assert
    ASSERT_TRUE(result. has_value());
    EXPECT_EQ(result->name, "New Event");
    EXPECT_EQ(result->status, model::EventStatus::Available);
    EXPECT_EQ(saved_event.name, "New Event");
}

TEST_F(EventServiceTest, CreateEvent_Success_SetsCreatedTime) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "Timed Event");
    
    EXPECT_CALL(*mock_repo_, findByName("Timed Event"))
        .WillOnce(Return(std::nullopt));
    
    model::Event saved_event;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_event));
    
    auto before_time = std::chrono::system_clock::now();
    
    // Act
    new_event.created_at = std::chrono::system_clock::now();
    auto result = event_service_->CreateEvent(new_event);
    
    auto after_time = std::chrono::system_clock::now();
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_GE(result->created_at, before_time);
    EXPECT_LE(result->created_at, after_time);
}

TEST_F(EventServiceTest, CreateEvent_Success_DefaultStatusAvailable) {
    // Arrange
    model::Event new_event;
    new_event.id = 0;
    new_event.name = "Default Status Event";
    new_event. status = model::EventStatus::Available;
    
    EXPECT_CALL(*mock_repo_, findByName("Default Status Event"))
        .WillOnce(Return(std::nullopt));
    
    model::Event saved_event;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_event));
    
    // Act
    auto result = event_service_->CreateEvent(new_event);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, model::EventStatus::Available);
}

TEST_F(EventServiceTest, CreateEvent_Success_CreateFrozenEvent) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "Frozen Event", model::EventStatus::Frozen);
    
    EXPECT_CALL(*mock_repo_, findByName("Frozen Event"))
        .WillOnce(Return(std::nullopt));
    
    model::Event saved_event;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_event));
    
    // Act
    auto result = event_service_->CreateEvent(new_event);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, model::EventStatus::Frozen);
}

TEST_F(EventServiceTest, CreateEvent_Failure_EmptyName) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "");
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, findByName(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    auto result = event_service_->CreateEvent(new_event);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(EventServiceTest, CreateEvent_Failure_DuplicateName) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "Existing Event");
    model::Event existing_event = CreateTestEvent(1, "Existing Event");
    
    EXPECT_CALL(*mock_repo_, findByName("Existing Event"))
        .WillOnce(Return(existing_event));
    
    // save 不应该被调用
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    auto result = event_service_->CreateEvent(new_event);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(EventServiceTest, CreateEvent_Success_MultipleDifferentEvents) {
    // Arrange
    model::Event event1 = CreateTestEvent(0, "Event 1");
    model::Event event2 = CreateTestEvent(0, "Event 2");
    
    EXPECT_CALL(*mock_repo_, findByName("Event 1"))
        .WillOnce(Return(std::nullopt));
    EXPECT_CALL(*mock_repo_, findByName("Event 2"))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(2);
    
    // Act
    auto result1 = event_service_->CreateEvent(event1);
    auto result2 = event_service_->CreateEvent(event2);
    
    // Assert
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2. has_value());
    EXPECT_EQ(result1->name, "Event 1");
    EXPECT_EQ(result2->name, "Event 2");
}

TEST_F(EventServiceTest, CreateEvent_Success_WithWhitespace) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "  Event With Spaces  ");
    
    EXPECT_CALL(*mock_repo_, findByName("  Event With Spaces  "))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    // Act
    auto result = event_service_->CreateEvent(new_event);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "  Event With Spaces  ");
}

// ==================== SetStatus Tests ====================

TEST_F(EventServiceTest, SetStatus_Success_ChangeToFrozen) {
    // Arrange
    const int event_id = 1;
    model::Event existing_event = CreateTestEvent(event_id, "Test Event", model::EventStatus::Available);
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        . WillOnce(Return(existing_event));
    
    EXPECT_CALL(*mock_repo_, setStatusById(event_id, model::EventStatus::Frozen))
        .WillOnce(Return(true));
    
    // Act
    bool result = event_service_->SetStatus(event_id, model::EventStatus::Frozen);
    
    // Assert
    EXPECT_TRUE(result);
}

TEST_F(EventServiceTest, SetStatus_Success_ChangeToAvailable) {
    // Arrange
    const int event_id = 1;
    model::Event existing_event = CreateTestEvent(event_id, "Test Event", model::EventStatus::Frozen);
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        .WillOnce(Return(existing_event));
    
    EXPECT_CALL(*mock_repo_, setStatusById(event_id, model::EventStatus::Available))
        .WillOnce(Return(true));
    
    // Act
    bool result = event_service_->SetStatus(event_id, model::EventStatus::Available);
    
    // Assert
    EXPECT_TRUE(result);
}

TEST_F(EventServiceTest, SetStatus_Success_SameStatus) {
    // Arrange
    const int event_id = 1;
    model::Event existing_event = CreateTestEvent(event_id, "Test Event", model::EventStatus::Available);
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        .WillOnce(Return(existing_event));
    
    EXPECT_CALL(*mock_repo_, setStatusById(event_id, model::EventStatus::Available))
        .WillOnce(Return(true));
    
    // Act
    bool result = event_service_->SetStatus(event_id, model::EventStatus::Available);
    
    // Assert
    EXPECT_TRUE(result);
}

TEST_F(EventServiceTest, SetStatus_Failure_EventNotFound) {
    // Arrange
    const int event_id = 999;
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        .WillOnce(Return(std::nullopt));
    
    // setStatusById 不应该被调用
    EXPECT_CALL(*mock_repo_, setStatusById(_, _))
        .Times(0);
    
    // Act
    bool result = event_service_->SetStatus(event_id, model::EventStatus::Frozen);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(EventServiceTest, SetStatus_Failure_InvalidEventId_Zero) {
    // Arrange
    const int invalid_event_id = 0;
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, setStatusById(_, _))
        .Times(0);
    
    // Act
    bool result = event_service_->SetStatus(invalid_event_id, model::EventStatus::Frozen);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(EventServiceTest, SetStatus_Failure_InvalidEventId_Negative) {
    // Arrange
    const int invalid_event_id = -1;
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, setStatusById(_, _))
        .Times(0);
    
    // Act
    bool result = event_service_->SetStatus(invalid_event_id, model::EventStatus::Frozen);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(EventServiceTest, SetStatus_Failure_RepositoryFails) {
    // Arrange
    const int event_id = 1;
    model::Event existing_event = CreateTestEvent(event_id, "Test Event");
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        . WillOnce(Return(existing_event));
    
    // 模拟 repository 操作失败
    EXPECT_CALL(*mock_repo_, setStatusById(event_id, model::EventStatus::Frozen))
        .WillOnce(Return(false));
    
    // Act
    bool result = event_service_->SetStatus(event_id, model::EventStatus::Frozen);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(EventServiceTest, SetStatus_Success_MultipleStatusChanges) {
    // Arrange
    const int event_id = 1;
    model::Event existing_event = CreateTestEvent(event_id, "Test Event", model::EventStatus::Available);
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        .Times(3)
        .WillRepeatedly(Return(existing_event));
    
    EXPECT_CALL(*mock_repo_, setStatusById(event_id, model::EventStatus::Frozen))
        . WillOnce(Return(true));
    EXPECT_CALL(*mock_repo_, setStatusById(event_id, model::EventStatus::Available))
        .Times(2)
        .WillRepeatedly(Return(true));
    
    // Act
    bool result1 = event_service_->SetStatus(event_id, model::EventStatus::Frozen);
    bool result2 = event_service_->SetStatus(event_id, model::EventStatus::Available);
    bool result3 = event_service_->SetStatus(event_id, model::EventStatus::Available);
    
    // Assert
    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(result3);
}

// ==================== GetEventById Tests ====================

TEST_F(EventServiceTest, GetEventById_Success) {
    // Arrange
    const int event_id = 1;
    model::Event expected_event = CreateTestEvent(event_id, "Test Event");
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        . WillOnce(Return(expected_event));
    
    // Act
    auto result = event_service_->QueryById(event_id);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, event_id);
    EXPECT_EQ(result->name, "Test Event");
}

TEST_F(EventServiceTest, GetEventById_Failure_NotFound) {
    // Arrange
    const int event_id = 999;
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result = event_service_->QueryById(event_id);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(EventServiceTest, GetEventById_Failure_InvalidId) {
    // Arrange
    const int invalid_id = 0;
    
    EXPECT_CALL(*mock_repo_, findById(_))
        . Times(0);
    
    // Act
    auto result = event_service_->QueryById(invalid_id);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

// ==================== Integration Tests ====================

TEST_F(EventServiceTest, Integration_CreateAndQuery) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "Shopping");
    
    // 创建事件
    EXPECT_CALL(*mock_repo_, findByName("Shopping"))
        . Times(2)
        .WillOnce(Return(std::nullopt))  // CreateEvent 检查
        .WillOnce(Return(CreateTestEvent(1, "Shopping")));  // QueryByName
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    // Act
    auto create_result = event_service_->CreateEvent(new_event);
    auto query_result = event_service_->QueryByName("Shopping");
    
    // Assert
    ASSERT_TRUE(create_result.has_value());
    ASSERT_TRUE(query_result.has_value());
    EXPECT_EQ(query_result->name, "Shopping");
}

TEST_F(EventServiceTest, Integration_CreateAndSetStatus) {
    // Arrange
    const int event_id = 1;
    model::Event new_event = CreateTestEvent(0, "Travel");
    
    EXPECT_CALL(*mock_repo_, findByName("Travel"))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    EXPECT_CALL(*mock_repo_, findById(event_id))
        .WillOnce(Return(CreateTestEvent(event_id, "Travel")));
    
    EXPECT_CALL(*mock_repo_, setStatusById(event_id, model::EventStatus::Frozen))
        .WillOnce(Return(true));
    
    // Act
    auto create_result = event_service_->CreateEvent(new_event);
    bool status_result = event_service_->SetStatus(event_id, model::EventStatus::Frozen);
    
    // Assert
    ASSERT_TRUE(create_result. has_value());
    EXPECT_TRUE(status_result);
}

TEST_F(EventServiceTest, Integration_CannotCreateDuplicateAfterCreation) {
    // Arrange
    model::Event new_event = CreateTestEvent(0, "Dining");
    
    // 第一次创建成功
    EXPECT_CALL(*mock_repo_, findByName("Dining"))
        .Times(2)
        .WillOnce(Return(std::nullopt))  // 第一次不存在
        .WillOnce(Return(CreateTestEvent(1, "Dining")));  // 第二次已存在
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);  // 只保存一次
    
    // Act
    auto result1 = event_service_->CreateEvent(new_event);
    auto result2 = event_service_->CreateEvent(new_event);
    
    // Assert
    EXPECT_TRUE(result1.has_value());
    EXPECT_FALSE(result2.has_value());
}