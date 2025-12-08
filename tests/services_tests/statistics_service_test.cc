#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "services/StatisticsService.h"
#include "data/irepositories.h"
#include "common/models.h"
#include <algorithm>

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

// Mock BillRepository
class MockBillRepository : public repo::IBillRepository {
public:
    MOCK_METHOD(void, save, (const model::Bill& b), (override));
    MOCK_METHOD(std::optional<model::Bill>, findById, (int id), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByEvent, (int ownerId, int eventId), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByEvent, (std::string& name), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByTime, (int ownerId, model::Timestamp from, model::Timestamp to), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByTime, (model::Timestamp from, model::Timestamp to), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByTimeInOrder, (model::Timestamp from, model::Timestamp to), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByTimeAndEventInOrder, (model::Timestamp from, model::Timestamp to), (override));
    MOCK_METHOD(std::vector<model::Bill>, queryByPhone, (const std::string& phone), (override));
    MOCK_METHOD(void, remove, (int id), (override));
};

class StatisticsServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_repo_ = std::make_shared<NiceMock<MockBillRepository>>();
        stats_service_ = std::make_unique<StatisticsService>(mock_repo_);
        
        // 设置基准时间
        base_time_ = std::chrono::system_clock::now();
    }

    void TearDown() override {
        stats_service_.reset();
        mock_repo_.reset();
    }

    // 辅助函数：创建测试账单
    model::Bill CreateTestBill(int id,
                               int owner_id,
                               int event_id,
                               const std::string& event_name,
                               double amount,
                               model::Timestamp created_at) {
        model::Bill bill;
        bill.id = id;
        bill.owner_id = owner_id;
        bill.event_id = event_id;
        bill.amount = amount;
        bill.created_at = created_at;
        bill.description = "Test Bill " + std::to_string(id);
        
        bill.event. id = event_id;
        bill.event.name = event_name;
        bill.event.status = model::EventStatus::Available;
        
        return bill;
    }

    // 辅助函数：验证账单是否按时间升序排列
    bool IsTimeAscending(const std::vector<model::Bill>& bills) {
        for (size_t i = 1; i < bills.size(); ++i) {
            if (bills[i].created_at < bills[i - 1].created_at) {
                return false;
            }
        }
        return true;
    }

    // 辅助函数：验证账单是否按时间和事件排序
    bool IsTimeAndEventOrdered(const std::vector<model::Bill>& bills) {
        for (size_t i = 1; i < bills.size(); ++i) {
            // 先按时间
            if (bills[i].created_at < bills[i - 1].created_at) {
                return false;
            }
            // 时间相同时按事件ID
            if (bills[i].created_at == bills[i - 1].created_at) {
                if (bills[i].event_id < bills[i - 1].event_id) {
                    return false;
                }
            }
        }
        return true;
    }

    std::shared_ptr<MockBillRepository> mock_repo_;
    std::unique_ptr<StatisticsService> stats_service_;
    model::Timestamp base_time_;
};

// ==================== QueryByTimeInOrder Tests ====================

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Success_MultipleResults_Ordered) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(24);
    auto to_time = base_time_;
    
    // 创建无序的账单列表（按时间）
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Shopping", 100.0, base_time_ - std::chrono::hours(20)),
        CreateTestBill(2, 2, 2, "Dining", 50.0, base_time_ - std::chrono::hours(15)),
        CreateTestBill(3, 1, 1, "Shopping", 200.0, base_time_ - std::chrono::hours(10)),
        CreateTestBill(4, 3, 3, "Transport", 30.0, base_time_ - std::chrono::hours(5))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 4);
    EXPECT_TRUE(IsTimeAscending(results));
    EXPECT_EQ(results[0].id, 1);
    EXPECT_EQ(results[1].id, 2);
    EXPECT_EQ(results[2]. id, 3);
    EXPECT_EQ(results[3].id, 4);
}

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Success_SingleResult) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(1);
    auto to_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Shopping", 100.0, base_time_ - std::chrono::minutes(30))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].id, 1);
}

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Success_NoResults) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(48);
    auto to_time = base_time_ - std::chrono::hours(24);
    
    std::vector<model::Bill> empty_bills;
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(empty_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Failure_InvalidTimeRange) {
    // Arrange
    auto from_time = base_time_;
    auto to_time = base_time_ - std::chrono::hours(24);  // to_time < from_time
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(_, _))
        .Times(0);
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Success_SameTime) {
    // Arrange
    auto same_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Shopping", 100.0, same_time)
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(same_time, same_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(same_time, same_time);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
}

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Success_LargeDataset) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(100);
    auto to_time = base_time_;
    
    // 创建 100 条记录
    std::vector<model::Bill> expected_bills;
    for (int i = 0; i < 100; ++i) {
        expected_bills. push_back(
            CreateTestBill(i + 1, 1, 1, "Event", 100.0, 
                          base_time_ - std::chrono::hours(100 - i))
        );
    }
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 100);
    EXPECT_TRUE(IsTimeAscending(results));
}

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Success_MultipleUsersData) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(24);
    auto to_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Shopping", 100.0, base_time_ - std::chrono::hours(20)),
        CreateTestBill(2, 2, 2, "Dining", 50.0, base_time_ - std::chrono::hours(18)),
        CreateTestBill(3, 3, 3, "Transport", 30.0, base_time_ - std::chrono::hours(16)),
        CreateTestBill(4, 1, 1, "Shopping", 200.0, base_time_ - std::chrono::hours(14))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 4);
    EXPECT_TRUE(IsTimeAscending(results));
    // 验证包含不同用户的数据
    EXPECT_EQ(results[0].owner_id, 1);
    EXPECT_EQ(results[1].owner_id, 2);
    EXPECT_EQ(results[2].owner_id, 3);
}

TEST_F(StatisticsServiceTest, QueryByTimeInOrder_Success_LongTimeRange) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(24 * 365);  // 一年
    auto to_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Event1", 100.0, base_time_ - std::chrono::hours(24 * 300)),
        CreateTestBill(2, 1, 2, "Event2", 200.0, base_time_ - std::chrono::hours(24 * 200)),
        CreateTestBill(3, 1, 3, "Event3", 300.0, base_time_ - std::chrono::hours(24 * 100))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 3);
    EXPECT_TRUE(IsTimeAscending(results));
}

// ==================== QueryByTimeAndEventInOrder Tests ====================

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_OrderedByTimeAndEvent) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(24);
    auto to_time = base_time_;
    
    auto same_time = base_time_ - std::chrono::hours(10);
    
    // 相同时间，不同事件ID
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Shopping", 100.0, base_time_ - std::chrono::hours(20)),
        CreateTestBill(2, 1, 1, "Shopping", 50.0, same_time),    // 相同时间，事件ID=1
        CreateTestBill(3, 1, 2, "Dining", 200.0, same_time),     // 相同时间，事件ID=2
        CreateTestBill(4, 1, 3, "Transport", 30.0, same_time),   // 相同时间，事件ID=3
        CreateTestBill(5, 1, 2, "Dining", 150.0, base_time_ - std::chrono::hours(5))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 5);
    EXPECT_TRUE(IsTimeAndEventOrdered(results));
    
    // 验证相同时间的记录按事件ID排序
    EXPECT_EQ(results[1].event_id, 1);
    EXPECT_EQ(results[2].event_id, 2);
    EXPECT_EQ(results[3].event_id, 3);
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_AllSameTime_DifferentEvents) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(1);
    auto to_time = base_time_;
    
    auto same_time = base_time_ - std::chrono::minutes(30);
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Event1", 100.0, same_time),
        CreateTestBill(2, 1, 2, "Event2", 200.0, same_time),
        CreateTestBill(3, 1, 3, "Event3", 300.0, same_time),
        CreateTestBill(4, 1, 4, "Event4", 400.0, same_time)
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 4);
    
    // 验证全部相同时间，按事件ID升序
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_EQ(results[i].created_at, results[0].created_at);
        EXPECT_GT(results[i].event_id, results[i - 1].event_id);
    }
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_SingleResult) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(1);
    auto to_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Shopping", 100.0, base_time_ - std::chrono::minutes(30))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].id, 1);
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_NoResults) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(48);
    auto to_time = base_time_ - std::chrono::hours(24);
    
    std::vector<model::Bill> empty_bills;
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(empty_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Failure_InvalidTimeRange) {
    // Arrange
    auto from_time = base_time_;
    auto to_time = base_time_ - std::chrono::hours(24);
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(_, _))
        .Times(0);
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_MixedTimeAndEvents) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(24);
    auto to_time = base_time_;
    
    auto time1 = base_time_ - std::chrono::hours(20);
    auto time2 = base_time_ - std::chrono::hours(15);
    auto time3 = base_time_ - std::chrono::hours(10);
    
    std::vector<model::Bill> expected_bills = {
        // 时间1
        CreateTestBill(1, 1, 2, "Event2", 100.0, time1),
        CreateTestBill(2, 1, 5, "Event5", 200.0, time1),
        // 时间2
        CreateTestBill(3, 1, 1, "Event1", 150.0, time2),
        CreateTestBill(4, 1, 3, "Event3", 250.0, time2),
        // 时间3
        CreateTestBill(5, 1, 4, "Event4", 300.0, time3)
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 5);
    EXPECT_TRUE(IsTimeAndEventOrdered(results));
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_LargeDataset) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(100);
    auto to_time = base_time_;
    
    std::vector<model::Bill> expected_bills;
    
    // 创建 50 个不同时间，每个时间 2 个不同事件
    for (int i = 0; i < 50; ++i) {
        auto time = base_time_ - std::chrono::hours(100 - i * 2);
        expected_bills.push_back(CreateTestBill(i * 2 + 1, 1, 1, "Event1", 100.0, time));
        expected_bills.push_back(CreateTestBill(i * 2 + 2, 1, 2, "Event2", 200.0, time));
    }
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 100);
    EXPECT_TRUE(IsTimeAndEventOrdered(results));
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_SameTimeRange) {
    // Arrange
    auto same_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Event1", 100.0, same_time),
        CreateTestBill(2, 1, 2, "Event2", 200.0, same_time)
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(same_time, same_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(same_time, same_time);
    
    // Assert
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].event_id, 1);
    EXPECT_EQ(results[1].event_id, 2);
}

TEST_F(StatisticsServiceTest, QueryByTimeAndEventInOrder_Success_AllDifferentEvents) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(10);
    auto to_time = base_time_;
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Event1", 100.0, base_time_ - std::chrono::hours(9)),
        CreateTestBill(2, 1, 2, "Event2", 200.0, base_time_ - std::chrono::hours(8)),
        CreateTestBill(3, 1, 3, "Event3", 300.0, base_time_ - std::chrono::hours(7)),
        CreateTestBill(4, 1, 4, "Event4", 400.0, base_time_ - std::chrono::hours(6))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 4);
    EXPECT_TRUE(IsTimeAndEventOrdered(results));
    
    // 所有时间都不同
    for (size_t i = 1; i < results.size(); ++i) {
        EXPECT_NE(results[i].created_at, results[i - 1].created_at);
    }
}

// ==================== Comparison Tests ====================

TEST_F(StatisticsServiceTest, Compare_BothMethods_SameTimeRange) {
    // Arrange
    auto from_time = base_time_ - std::chrono::hours(24);
    auto to_time = base_time_;
    
    auto same_time = base_time_ - std::chrono::hours(10);
    
    std::vector<model::Bill> bills_by_time = {
        CreateTestBill(1, 1, 3, "Event3", 100.0, base_time_ - std::chrono::hours(20)),
        CreateTestBill(2, 1, 1, "Event1", 50.0, same_time),
        CreateTestBill(3, 1, 2, "Event2", 200.0, same_time),
        CreateTestBill(4, 1, 4, "Event4", 30.0, base_time_ - std::chrono::hours(5))
    };
    
    std::vector<model::Bill> bills_by_time_and_event = {
        CreateTestBill(1, 1, 3, "Event3", 100.0, base_time_ - std::chrono::hours(20)),
        CreateTestBill(2, 1, 1, "Event1", 50.0, same_time),    // 相同时间，事件ID小的在前
        CreateTestBill(3, 1, 2, "Event2", 200.0, same_time),
        CreateTestBill(4, 1, 4, "Event4", 30.0, base_time_ - std::chrono::hours(5))
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(bills_by_time));
    
    EXPECT_CALL(*mock_repo_, queryByTimeAndEventInOrder(from_time, to_time))
        .WillOnce(Return(bills_by_time_and_event));
    
    // Act
    auto results1 = stats_service_->QueryByTimeInOrder(from_time, to_time);
    auto results2 = stats_service_->QueryByTimeAndEventInOrder(from_time, to_time);
    
    // Assert
    EXPECT_EQ(results1.size(), results2.size());
    EXPECT_TRUE(IsTimeAscending(results1));
    EXPECT_TRUE(IsTimeAndEventOrdered(results2));
}

// ==================== Edge Cases ====================

TEST_F(StatisticsServiceTest, EdgeCase_VeryShortTimeRange) {
    // Arrange
    auto from_time = base_time_;
    auto to_time = base_time_ + std::chrono::seconds(1);
    
    std::vector<model::Bill> expected_bills = {
        CreateTestBill(1, 1, 1, "Event", 100.0, base_time_)
    };
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(expected_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
}

TEST_F(StatisticsServiceTest, EdgeCase_FutureTimeRange) {
    // Arrange
    auto from_time = base_time_ + std::chrono::hours(24);
    auto to_time = base_time_ + std::chrono::hours(48);
    
    std::vector<model::Bill> empty_bills;
    
    EXPECT_CALL(*mock_repo_, queryByTimeInOrder(from_time, to_time))
        .WillOnce(Return(empty_bills));
    
    // Act
    auto results = stats_service_->QueryByTimeInOrder(from_time, to_time);
    
    // Assert
    EXPECT_TRUE(results.empty());
}