#pragma once

#include "../common/models.h"
#include <vector>
#include <optional>
#include <memory>
#include <map>

// 声明了仓库接口，待数据层实现
namespace repo {

    struct IUserRepository {
        virtual ~IUserRepository() = default;
        virtual void save(const model::User& u) = 0;

        virtual std::optional<model::User> findById(int id) = 0; 

        virtual std::optional<model::User> queryByPhone(const std::string& phone) = 0; // 仅管理员可用
        virtual std::vector<model::User> queryByPhonePartial(const std::string& partial) = 0; // 仅管理员可用

        virtual bool setBalanceByPhone(const std::string& phone) = 0; // 仅管理员可用
    };

    struct IBillRepository {
        virtual ~IBillRepository() = default;
        virtual void save(const model::Bill& b) = 0;

        virtual std::optional<model::Bill> findById(int id) = 0;

        virtual std::vector<model::Bill> queryByEvent(int ownerId, int eventId) = 0;
        virtual std::vector<model::Bill> queryByEvent(std::string& name) = 0; // 仅管理员可用

        virtual std::vector<model::Bill> queryByTime(int ownerId, model::Timestamp from, model::Timestamp to) = 0;
        virtual std::vector<model::Bill> queryByTime(model::Timestamp from, model::Timestamp to) = 0; // 仅管理员可用

        virtual std::vector<model::Bill> queryByPhone(const std::string& phone) = 0; // 仅管理员可用
        virtual void remove(int id) = 0;
    };

    struct IEventRepository {
        virtual ~IEventRepository() = default;
        virtual void save(const model::Event& e) = 0; // 仅管理员可用

        virtual std::optional<model::Event> findById(int Id) = 0; // 仅管理员可用
        virtual std::optional<model::Event> findByName(const std::string& name) = 0; // 仅管理员可用

        virtual bool setStatusById(int id, model::EventStatus status) = 0; // 仅管理员可用
    };

    struct IAnnotationRepository {
        virtual ~IAnnotationRepository() = default;
        virtual void save(const model::Annotation& a) = 0; // 仅管理员可用

        virtual std::optional<model::Annotation> findById(int Id) = 0; 
    };

}