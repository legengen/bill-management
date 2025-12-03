#pragma once

#include "../common/models.h"
#include <vector>
#include <optional>
#include <memory>
#include <map>

// 声明了仓库接口，待数据层实现
namespace repo {
    using namespace model;

    struct IUserRepository {
        virtual ~IUserRepository() = default;
        virtual void save(const User& u) = 0;
        virtual std::optional<User> findById(int id) = 0; 
        virtual User& queryByPhone(const std::string& phone) = 0; // 仅管理员可用
        virtual bool setBalanceByPhone(const std::string& phone) = 0; // 仅管理员可用
    };

    struct IBillRepository {
        virtual ~IBillRepository() = default;
        virtual void save(const Bill& b) = 0;
        virtual std::optional<Bill> findById(int id) = 0;
        virtual std::vector<Bill> queryByEvent(int ownerId, int eventId) = 0;
        virtual std::vector<Bill> queryByEvent(std::string& name) = 0; // 仅管理员可用

        virtual std::vector<Bill> queryByTime(int ownerId, Timestamp from, Timestamp to) = 0;
        virtual std::vector<Bill> queryByTime(Timestamp from, Timestamp to) = 0; // 仅管理员可用

        virtual std::vector<Bill> queryByPhone(const std::string& phone) = 0; // 仅管理员可用
        virtual void remove(int id) = 0;
    };

    struct IEventRepository {
        virtual ~IEventRepository() = default;
        virtual void save(const Event& e) = 0; // 仅管理员可用
        virtual Event& findById(int Id) = 0; // 仅管理员可用
        virtual Event& findByName(int name) = 0; // 仅管理员可用
        virtual bool setStatusById(int Id) = 0; // 仅管理员可用
    };

    struct IAnnotationRepository {
        virtual ~IAnnotationRepository() = default;
        virtual void save(const Annotation& a) = 0; // 仅管理员可用
        virtual Annotation& findById(int Id) = 0; 
    };

}