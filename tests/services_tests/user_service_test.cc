#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <UserService.h>
#include <irepositories.h>
#include <models.h>

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::SaveArg;

// Mock UserRepository
class MockUserRepository : public repo::IUserRepository {
public:
    MOCK_METHOD(void, save, (const model::User& u), (override));
    MOCK_METHOD(std::optional<model::User>, findById, (int id), (override));
    MOCK_METHOD(std::optional<model::User>, queryByPhone, (const std::string& phone), (override));
    MOCK_METHOD(std::vector<model::User>, queryByPhonePartial, (const std::string& partial), (override));
    MOCK_METHOD(bool, setBalanceByPhone, (const std::string& phone), (override));
};

class UserServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_repo_ = std::make_shared<NiceMock<MockUserRepository>>();
        user_service_ = std::make_unique<UserService>(mock_repo_);
    }

    void TearDown() override {
        user_service_.reset();
        mock_repo_.reset();
    }

    // 辅助函数：创建测试用户
    model::User CreateTestUser(int id, 
                                const std::string& phone,
                                const std::string& username = "testuser",
                                const std::string& role = "user",
                                double balance = 0.0) {
        model::User user;
        user. id = id;
        user. phone = phone;
        user. username = username;
        user. password = "password123";
        user.role = role;
        user.balance = balance;
        user.created_at = std::chrono::system_clock::now();
        return user;
    }

    std::shared_ptr<MockUserRepository> mock_repo_;
    std::unique_ptr<UserService> user_service_;
};

// ==================== GetUser Tests ====================

TEST_F(UserServiceTest, GetUser_Success_ValidUserId) {
    // Arrange
    const int user_id = 1;
    model::User expected_user = CreateTestUser(1, "13800138000", "alice", "user", 100.0);
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(expected_user));
    
    // Act
    auto result = user_service_->GetUser(user_id);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->phone, "13800138000");
    EXPECT_EQ(result->username, "alice");
    EXPECT_DOUBLE_EQ(result->balance, 100.0);
}

TEST_F(UserServiceTest, GetUser_Failure_UserNotFound) {
    // Arrange
    const int user_id = 999;
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result = user_service_->GetUser(user_id);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(UserServiceTest, GetUser_Failure_InvalidUserId_Zero) {
    // Arrange
    const int invalid_user_id = 0;
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    
    // Act
    auto result = user_service_->GetUser(invalid_user_id);
    
    // Assert
    EXPECT_FALSE(result. has_value());
}

TEST_F(UserServiceTest, GetUser_Failure_InvalidUserId_Negative) {
    // Arrange
    const int invalid_user_id = -1;
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, findById(_))
        . Times(0);
    
    // Act
    auto result = user_service_->GetUser(invalid_user_id);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(UserServiceTest, GetUser_Success_MultipleCallsSameUser) {
    // Arrange
    const int user_id = 1;
    model::User expected_user = CreateTestUser(1, "13800138000", "bob");
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        . Times(2)
        .WillRepeatedly(Return(expected_user));
    
    // Act
    auto result1 = user_service_->GetUser(user_id);
    auto result2 = user_service_->GetUser(user_id);
    
    // Assert
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result1->id, result2->id);
    EXPECT_EQ(result1->phone, result2->phone);
}

TEST_F(UserServiceTest, GetUser_Success_DifferentUsers) {
    // Arrange
    model::User user1 = CreateTestUser(1, "13800138000", "alice");
    model::User user2 = CreateTestUser(2, "13812345678", "bob");
    
    EXPECT_CALL(*mock_repo_, findById(1))
        .WillOnce(Return(user1));
    EXPECT_CALL(*mock_repo_, findById(2))
        .WillOnce(Return(user2));
    
    // Act
    auto result1 = user_service_->GetUser(1);
    auto result2 = user_service_->GetUser(2);
    
    // Assert
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result1->username, "alice");
    EXPECT_EQ(result2->username, "bob");
}

// ==================== QueryUserByPhone Tests ====================

TEST_F(UserServiceTest, QueryUserByPhone_Success_MultipleResults) {
    // Arrange
    const std::string phone_partial = "138";
    
    std::vector<model::User> expected_users = {
        CreateTestUser(1, "13800138000", "alice"),
        CreateTestUser(2, "13812345678", "bob"),
        CreateTestUser(3, "13898765432", "charlie")
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(phone_partial))
        .WillOnce(Return(expected_users));
    
    // Act
    auto results = user_service_->QueryUserByPhone(phone_partial);
    
    // Assert
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0].phone, "13800138000");
    EXPECT_EQ(results[0].username, "alice");
    EXPECT_EQ(results[1].phone, "13812345678");
    EXPECT_EQ(results[1].username, "bob");
    EXPECT_EQ(results[2].phone, "13898765432");
    EXPECT_EQ(results[2].username, "charlie");
}

TEST_F(UserServiceTest, QueryUserByPhone_Success_SingleResult) {
    // Arrange
    const std::string phone_partial = "13800138";
    
    std::vector<model::User> expected_users = {
        CreateTestUser(1, "13800138000", "alice")
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(phone_partial))
        .WillOnce(Return(expected_users));
    
    // Act
    auto results = user_service_->QueryUserByPhone(phone_partial);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].phone, "13800138000");
    EXPECT_EQ(results[0]. username, "alice");
}

TEST_F(UserServiceTest, QueryUserByPhone_Success_NoResults) {
    // Arrange
    const std::string phone_partial = "999";
    
    std::vector<model::User> empty_results;
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(phone_partial))
        .WillOnce(Return(empty_results));
    
    // Act
    auto results = user_service_->QueryUserByPhone(phone_partial);
    
    // Assert
    EXPECT_TRUE(results.empty());
    EXPECT_EQ(results.size(), 0);
}

TEST_F(UserServiceTest, QueryUserByPhone_Failure_EmptyPhone) {
    // Arrange
    const std::string empty_phone = "";
    
    // 不应该调用 repository
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(_))
        .Times(0);
    
    // Act
    auto results = user_service_->QueryUserByPhone(empty_phone);
    
    // Assert
    EXPECT_TRUE(results.empty());
}

TEST_F(UserServiceTest, QueryUserByPhone_Success_PartialMatchBeginning) {
    // Arrange
    const std::string phone_partial = "138";
    
    std::vector<model::User> expected_users = {
        CreateTestUser(1, "13800001111", "user1"),
        CreateTestUser(2, "13811112222", "user2")
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(phone_partial))
        .WillOnce(Return(expected_users));
    
    // Act
    auto results = user_service_->QueryUserByPhone(phone_partial);
    
    // Assert
    ASSERT_EQ(results.size(), 2);
    for (const auto& user : results) {
        EXPECT_TRUE(user.phone.find(phone_partial) != std::string::npos);
    }
}

TEST_F(UserServiceTest, QueryUserByPhone_Success_PartialMatchMiddle) {
    // Arrange
    const std::string phone_partial = "8888";
    
    std::vector<model::User> expected_users = {
        CreateTestUser(1, "13088889999", "user1"),
        CreateTestUser(2, "15188887777", "user2")
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(phone_partial))
        .WillOnce(Return(expected_users));
    
    // Act
    auto results = user_service_->QueryUserByPhone(phone_partial);
    
    // Assert
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].phone, "13088889999");
    EXPECT_EQ(results[1].phone, "15188887777");
}

TEST_F(UserServiceTest, QueryUserByPhone_Success_PartialMatchEnd) {
    // Arrange
    const std::string phone_partial = "0000";
    
    std::vector<model::User> expected_users = {
        CreateTestUser(1, "13800000000", "user1")
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(phone_partial))
        .WillOnce(Return(expected_users));
    
    // Act
    auto results = user_service_->QueryUserByPhone(phone_partial);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].phone, "13800000000");
}

TEST_F(UserServiceTest, QueryUserByPhone_Success_CompletePhoneNumber) {
    // Arrange
    const std::string complete_phone = "13800138000";
    
    std::vector<model::User> expected_users = {
        CreateTestUser(1, "13800138000", "alice")
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(complete_phone))
        .WillOnce(Return(expected_users));
    
    // Act
    auto results = user_service_->QueryUserByPhone(complete_phone);
    
    // Assert
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].phone, complete_phone);
}

// ==================== SetBalance Tests ====================

TEST_F(UserServiceTest, SetBalance_Success_UpdateBalance) {
    // Arrange
    const int user_id = 1;
    const double new_balance = 150.75;
    
    model::User existing_user = CreateTestUser(1, "13800138000", "alice", "user", 100.0);
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        . WillOnce(Return(existing_user));
    
    model::User saved_user;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_user));
    
    // Act
    user_service_->SetBalance(user_id, new_balance);
    
    // Assert
    EXPECT_DOUBLE_EQ(saved_user.balance, new_balance);
    EXPECT_EQ(saved_user.id, user_id);
    EXPECT_EQ(saved_user.phone, "13800138000");
}

TEST_F(UserServiceTest, SetBalance_Success_SetBalanceToZero) {
    // Arrange
    const int user_id = 1;
    const double new_balance = 0.0;
    
    model::User existing_user = CreateTestUser(1, "13800138000", "alice", "user", 100.0);
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(existing_user));
    
    model::User saved_user;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_user));
    
    // Act
    user_service_->SetBalance(user_id, new_balance);
    
    // Assert
    EXPECT_DOUBLE_EQ(saved_user.balance, 0.0);
}

TEST_F(UserServiceTest, SetBalance_Success_IncreaseLargeAmount) {
    // Arrange
    const int user_id = 1;
    const double new_balance = 9999999.99;
    
    model::User existing_user = CreateTestUser(1, "13800138000", "alice", "user", 0.0);
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(existing_user));
    
    model::User saved_user;
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1)
        .WillOnce(SaveArg<0>(&saved_user));
    
    // Act
    user_service_->SetBalance(user_id, new_balance);
    
    // Assert
    EXPECT_DOUBLE_EQ(saved_user.balance, 9999999.99);
}

TEST_F(UserServiceTest, SetBalance_Failure_UserNotFound) {
    // Arrange
    const int user_id = 999;
    const double new_balance = 100.0;
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(std::nullopt));
    
    // save 不应该被调用
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    user_service_->SetBalance(user_id, new_balance);
    
    // Assert - 无异常即通过
}

TEST_F(UserServiceTest, SetBalance_Failure_InvalidUserId_Zero) {
    // Arrange
    const int invalid_user_id = 0;
    const double new_balance = 100.0;
    
    // repository 方法都不应该被调用
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    user_service_->SetBalance(invalid_user_id, new_balance);
    
    // Assert - 无异常即通过
}

TEST_F(UserServiceTest, SetBalance_Failure_InvalidUserId_Negative) {
    // Arrange
    const int invalid_user_id = -1;
    const double new_balance = 100.0;
    
    // repository 方法都不应该被调用
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    user_service_->SetBalance(invalid_user_id, new_balance);
    
    // Assert - 无异常即通过
}

TEST_F(UserServiceTest, SetBalance_Failure_NegativeAmount) {
    // Arrange
    const int user_id = 1;
    const double negative_balance = -100.0;
    
    // repository 方法都不应该被调用
    EXPECT_CALL(*mock_repo_, findById(_))
        .Times(0);
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    user_service_->SetBalance(user_id, negative_balance);
    
    // Assert - 无异常即通过
}

TEST_F(UserServiceTest, SetBalance_Success_MultipleUpdates) {
    // Arrange
    const int user_id = 1;
    model::User existing_user = CreateTestUser(1, "13800138000", "alice", "user", 100.0);
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .Times(3)
        .WillRepeatedly(Return(existing_user));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(3);
    
    // Act
    user_service_->SetBalance(user_id, 200.0);
    user_service_->SetBalance(user_id, 300.0);
    user_service_->SetBalance(user_id, 400.0);
    
    // Assert - 验证调用次数即可
}

TEST_F(UserServiceTest, SetBalance_Success_PreserveOtherFields) {
    // Arrange
    const int user_id = 1;
    const double new_balance = 500.0;
    
    model::User existing_user = CreateTestUser(1, "13800138000", "alice", "admin", 100.0);
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(existing_user));
    
    model::User saved_user;
    EXPECT_CALL(*mock_repo_, save(_))
        . Times(1)
        .WillOnce(SaveArg<0>(&saved_user));
    
    // Act
    user_service_->SetBalance(user_id, new_balance);
    
    // Assert - 验证其他字段未被修改
    EXPECT_EQ(saved_user.id, 1);
    EXPECT_EQ(saved_user.phone, "13800138000");
    EXPECT_EQ(saved_user.username, "alice");
    EXPECT_EQ(saved_user.role, "admin");
    EXPECT_DOUBLE_EQ(saved_user.balance, new_balance);
}

TEST_F(UserServiceTest, SetBalance_EdgeCase_VerySmallAmount) {
    // Arrange
    const int user_id = 1;
    const double small_balance = 0.01;
    
    model::User existing_user = CreateTestUser(1, "13800138000", "alice");
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        . WillOnce(Return(existing_user));
    
    model::User saved_user;
    EXPECT_CALL(*mock_repo_, save(_))
        . Times(1)
        .WillOnce(SaveArg<0>(&saved_user));
    
    // Act
    user_service_->SetBalance(user_id, small_balance);
    
    // Assert
    EXPECT_DOUBLE_EQ(saved_user.balance, 0.01);
}

// ==================== Integration Tests ====================

TEST_F(UserServiceTest, Integration_GetUserAndSetBalance) {
    // Arrange
    const int user_id = 1;
    model::User user = CreateTestUser(1, "13800138000", "alice", "user", 100.0);
    
    // 第一次获取用户
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .Times(2)
        .WillRepeatedly(Return(user));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    // Act
    auto result1 = user_service_->GetUser(user_id);
    ASSERT_TRUE(result1.has_value());
    EXPECT_DOUBLE_EQ(result1->balance, 100.0);
    
    user_service_->SetBalance(user_id, 200.0);
    
    // Assert - 验证调用序列
}

TEST_F(UserServiceTest, Integration_QueryAndSetBalance) {
    // Arrange
    const std::string phone_partial = "138";
    const int user_id = 1;
    
    std::vector<model::User> users = {
        CreateTestUser(1, "13800138000", "alice", "user", 100.0)
    };
    
    EXPECT_CALL(*mock_repo_, queryByPhonePartial(phone_partial))
        .WillOnce(Return(users));
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(users[0]));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    // Act
    auto results = user_service_->QueryUserByPhone(phone_partial);
    ASSERT_EQ(results.size(), 1);
    
    user_service_->SetBalance(results[0].id, 300.0);
    
    // Assert - 验证调用序列
}