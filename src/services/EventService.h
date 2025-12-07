#pragma once
#include <irepositories.h>

class EventService{
public:
    explicit EventService(std::shared_ptr<repo::IEventRepository> event_repo_):
        event_repository_(event_repo_) {}

    std::optional<model::Event> QueryByName(const std::string& name);
    std::optional<model::Event> QueryById(int event_id);
    std::optional<model::Event> CreateEvent(model::Event& e);
    bool SetStatus(int event_id, int status);
private:
    std::shared_ptr<repo::IEventRepository> event_repository_;
};

