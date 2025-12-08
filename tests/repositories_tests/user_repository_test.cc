#include "DatabaseTestBase.h"

class UserRepositoryTest : public DatabaseTestBase {};

// ==================== save 测试 ====================

TEST_F(UserRepositoryTest, Save_NewUser_Success) {
    // Arrange
    auto user = CreateUser("13900000001", "NewUser");
    
    // Act
    user_repo_->save(user);
    
    // Assert
    auto saved = user_repo_->queryByPhone("13900000001");
    ASSERT_TRUE(saved.has_value());
    EXPECT_EQ(saved->username, "NewUser");
    EXPECT_EQ(saved->phone, "13900000001");
    EXPECT_GT(saved->id, 0);
}

TEST_F(UserRepositoryTest, Save_UpdateExisting_Success) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    ASSERT_TRUE(user.has_value());
    
    // Act
    user->username = "UpdatedName";
    user->balance = 2000.0;
    user_repo_->save(*user);
    
    // Assert
    auto updated = user_repo_->queryByPhone("13800000001");
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->username, "UpdatedName");
    EXPECT_DOUBLE_EQ(updated->balance, 2000.0);
}

// ==================== findById 测试 ====================

TEST_F(UserRepositoryTest, FindById_Exists_ReturnsUser) {
    // Arrange
    auto user = user_repo_->queryByPhone("13800000001");
    ASSERT_TRUE(user.has_value());
    int user_id = user->id;
    
    // Act
    auto found = user_repo_->findById(user_id);
    
    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->phone, "13800000001");
}

TEST_F(UserRepositoryTest, FindById_NotExists_ReturnsNullopt) {
    // Act
    auto found = user_repo_->findById(99999);
    
    // Assert
    EXPECT_FALSE(found.has_value());
}

// ==================== queryByPhone 测试 ====================

TEST_F(UserRepositoryTest, QueryByPhone_Exists_ReturnsUser) {
    // Act
    auto found = user_repo_->queryByPhone("13800000001");
    
    // Assert
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->username, "TestUser1");
}

TEST_F(UserRepositoryTest, QueryByPhone_NotExists_ReturnsNullopt) {
    // Act
    auto found = user_repo_->queryByPhone("99999999999");
    
    // Assert
    EXPECT_FALSE(found. has_value());
}

// ==================== searchByPhonePartial 测试 ====================

TEST_F(UserRepositoryTest, SearchByPhonePartial_MatchesMultiple) {
    // Act
    auto results = user_repo_->queryByPhonePartial("138");
    
    // Assert
    EXPECT_EQ(results.size(), 2);
}

TEST_F(UserRepositoryTest, SearchByPhonePartial_MatchesOne) {
    // Act
    auto results = user_repo_->queryByPhonePartial("0001");
    
    // Assert
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]. username, "TestUser1");
}

TEST_F(UserRepositoryTest, SearchByPhonePartial_NoMatch) {
    // Act
    auto results = user_repo_->queryByPhonePartial("999");
    
    // Assert
    EXPECT_TRUE(results.empty());
}

// ==================== 集成测试 ====================

TEST_F(UserRepositoryTest, Integration_CreateQueryUpdate) {
    // 1. 创建用户
    auto new_user = CreateUser("13911111111", "IntegrationUser", "pass123");
    user_repo_->save(new_user);
    
    // 2. 查询用户
    auto found = user_repo_->queryByPhone("13911111111");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->username, "IntegrationUser");
    
    // 3. 更新用户
    found->balance = 500.0;
    found->username = "UpdatedIntegrationUser";
    user_repo_->save(*found);
    
    // 4. 再次查询验证
    auto updated = user_repo_->findById(found->id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->username, "UpdatedIntegrationUser");
    EXPECT_DOUBLE_EQ(updated->balance, 500.0);
}