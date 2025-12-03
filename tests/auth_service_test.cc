#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../src/services/AuthService.h"
#include "../data/irepositories.h"
#include "../common/models.h"

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

// Mock UserRepository for testing
class MockUserRepository : public repo::IUserRepository {
public:
    MOCK_METHOD(void, save, (const model::User& u), (override));
    MOCK_METHOD(std::optional<model::User>, findById, (int id), (override));
    MOCK_METHOD(std::optional<model::User>, queryByPhone, (const std::string& phone), (override));
    MOCK_METHOD(std::vector<model::User>, queryByPhonePartial, (const std::string& partial), (override));
    MOCK_METHOD(bool, setBalanceByPhone, (const std::string& phone), (override));
};

class AuthServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_repo_ = std::make_shared<NiceMock<MockUserRepository>>();
        auth_service_ = std::make_unique<AuthService>(mock_repo_);
    }

    void TearDown() override {
        auth_service_.reset();
        mock_repo_.reset();
    }

    std::shared_ptr<MockUserRepository> mock_repo_;
    std::unique_ptr<AuthService> auth_service_;
};

// ==================== Login Tests ====================

TEST_F(AuthServiceTest, Login_Success_WithValidCredentials) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string password = "password123";
    
    model::User expected_user;
    expected_user.id = 1;
    expected_user.phone = phone;
    expected_user.username = "testuser";
    expected_user. password = password; 
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        . WillOnce(Return(expected_user));
    
    // Act
    auto result = auth_service_->Login(phone, password);
    
    // Assert
    ASSERT_TRUE(result. has_value());
    EXPECT_EQ(result->id, 1);
    EXPECT_EQ(result->phone, phone);
    EXPECT_EQ(result->username, "testuser");
}

TEST_F(AuthServiceTest, Login_Failure_UserNotFound) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string password = "password123";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result = auth_service_->Login(phone, password);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(AuthServiceTest, Login_Failure_WrongPassword) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string correct_password = "password123";
    const std::string wrong_password = "wrongpassword";
    
    model::User existing_user;
    existing_user.id = 1;
    existing_user.phone = phone;
    existing_user.password = correct_password;
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(existing_user));
    
    // Act
    auto result = auth_service_->Login(phone, wrong_password);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(AuthServiceTest, Login_WithEmptyPhone) {
    // Arrange
    const std::string empty_phone = "";
    const std::string password = "password123";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(empty_phone))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result = auth_service_->Login(empty_phone, password);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(AuthServiceTest, Login_WithEmptyPassword) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string empty_password = "";
    
    model::User existing_user;
    existing_user.id = 1;
    existing_user.phone = phone;
    existing_user.password = "password123";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(existing_user));
    
    // Act
    auto result = auth_service_->Login(phone, empty_password);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

// ==================== Register Tests ====================

TEST_F(AuthServiceTest, Register_Success_NewUser) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string username = "newuser";
    const std::string password = "password123";
    
    // User doesn't exist
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(std::nullopt));
    
    // Expect save to be called
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    // Act
    auto result = auth_service_->Register(phone, username, password);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->phone, phone);
    EXPECT_EQ(result->username, username);
}

TEST_F(AuthServiceTest, Register_Failure_PhoneAlreadyExists) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string username = "newuser";
    const std::string password = "password123";
    
    model::User existing_user;
    existing_user.id = 1;
    existing_user.phone = phone;
    existing_user.username = "existinguser";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(existing_user));
    
    // save should NOT be called
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    auto result = auth_service_->Register(phone, username, password);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(AuthServiceTest, Register_WithEmptyPhone) {
    // Arrange
    const std::string empty_phone = "";
    const std::string username = "newuser";
    const std::string password = "password123";
    
    // Act
    auto result = auth_service_->Register(empty_phone, username, password);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(AuthServiceTest, Register_WithEmptyUsername) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string empty_username = "";
    const std::string password = "password123";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result = auth_service_->Register(phone, empty_username, password);
    
    // Assert
    // Depending on your business logic, this might succeed or fail
    // Adjust based on your requirements
    EXPECT_FALSE(result.has_value());
}

TEST_F(AuthServiceTest, Register_WithEmptyPassword) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string username = "newuser";
    const std::string empty_password = "";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(std::nullopt));
    
    // Act
    auto result = auth_service_->Register(phone, username, empty_password);
    
    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(AuthServiceTest, Register_WithInvalidPhoneFormat) {
    // Arrange
    const std::string invalid_phone = "12345"; // Too short
    const std::string username = "newuser";
    const std::string password = "password123";
    
    // Act
    auto result = auth_service_->Register(invalid_phone, username, password);
    
    // Assert
    // Depending on validation logic
    EXPECT_FALSE(result.has_value());
}

// ==================== ResetPassword Tests ====================

TEST_F(AuthServiceTest, ResetPassword_Success) {
    // Arrange
    const int user_id = 1;
    const std::string old_password = "oldpassword";
    const std::string new_password = "newpassword";
    
    model::User existing_user;
    existing_user.id = user_id;
    existing_user. password = old_password;
    existing_user.phone = "13800138000";
    existing_user.username = "testuser";
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(existing_user));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    // Act
    bool result = auth_service_->ResetPassword(user_id, old_password, new_password);
    
    // Assert
    EXPECT_TRUE(result);
}

TEST_F(AuthServiceTest, ResetPassword_Failure_UserNotFound) {
    // Arrange
    const int user_id = 999;
    const std::string old_password = "oldpassword";
    const std::string new_password = "newpassword";
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    bool result = auth_service_->ResetPassword(user_id, old_password, new_password);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(AuthServiceTest, ResetPassword_Failure_WrongOldPassword) {
    // Arrange
    const int user_id = 1;
    const std::string correct_old_password = "oldpassword";
    const std::string wrong_old_password = "wrongpassword";
    const std::string new_password = "newpassword";
    
    model::User existing_user;
    existing_user.id = user_id;
    existing_user.password = correct_old_password;
    existing_user.phone = "13800138000";
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(existing_user));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(0);
    
    // Act
    bool result = auth_service_->ResetPassword(user_id, wrong_old_password, new_password);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(AuthServiceTest, ResetPassword_WithEmptyNewPassword) {
    // Arrange
    const int user_id = 1;
    const std::string old_password = "oldpassword";
    const std::string empty_new_password = "";
    
    model::User existing_user;
    existing_user.id = user_id;
    existing_user. password = old_password;
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(existing_user));
    
    // Act
    bool result = auth_service_->ResetPassword(user_id, old_password, empty_new_password);
    
    // Assert
    EXPECT_FALSE(result);
}

TEST_F(AuthServiceTest, ResetPassword_SameOldAndNewPassword) {
    // Arrange
    const int user_id = 1;
    const std::string password = "samepassword";
    
    model::User existing_user;
    existing_user.id = user_id;
    existing_user.password = password;
    
    EXPECT_CALL(*mock_repo_, findById(user_id))
        .WillOnce(Return(existing_user));
    
    // Act
    bool result = auth_service_->ResetPassword(user_id, password, password);
    
    // Assert
    // Depending on your business logic, this might succeed or fail
    // Adjust based on your requirements
    EXPECT_FALSE(result);
}

TEST_F(AuthServiceTest, ResetPassword_WithInvalidUserId) {
    // Arrange
    const int invalid_user_id = -1;
    const std::string old_password = "oldpassword";
    const std::string new_password = "newpassword";
    
    EXPECT_CALL(*mock_repo_, findById(invalid_user_id))
        .WillOnce(Return(std::nullopt));
    
    // Act
    bool result = auth_service_->ResetPassword(invalid_user_id, old_password, new_password);
    
    // Assert
    EXPECT_FALSE(result);
}

// ==================== Edge Cases ====================

TEST_F(AuthServiceTest, Login_WithSpecialCharactersInPassword) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string password = "p@ssw0rd! #$%";
    
    model::User existing_user;
    existing_user.id = 1;
    existing_user.phone = phone;
    existing_user.password = password;
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(existing_user));
    
    // Act
    auto result = auth_service_->Login(phone, password);
    
    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, 1);
}

TEST_F(AuthServiceTest, Register_WithSpecialCharactersInUsername) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string username = "user_name-123";
    const std::string password = "password123";
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .WillOnce(Return(std::nullopt));
    
    EXPECT_CALL(*mock_repo_, save(_))
        .Times(1);
    
    // Act
    auto result = auth_service_->Register(phone, username, password);
    
    // Assert
    ASSERT_TRUE(result. has_value());
    EXPECT_EQ(result->username, username);
}

TEST_F(AuthServiceTest, Login_MultipleCallsSameUser) {
    // Arrange
    const std::string phone = "13800138000";
    const std::string password = "password123";
    
    model::User existing_user;
    existing_user.id = 1;
    existing_user.phone = phone;
    existing_user. password = password;
    
    EXPECT_CALL(*mock_repo_, queryByPhone(phone))
        .Times(2)
        .WillRepeatedly(Return(existing_user));
    
    // Act
    auto result1 = auth_service_->Login(phone, password);
    auto result2 = auth_service_->Login(phone, password);
    
    // Assert
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result1->id, result2->id);
}

// Main function to run all tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}