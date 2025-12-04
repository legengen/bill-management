#include "EventService.h"

std::optional<model::Event> EventService::QueryByName(const std::string& name) {
    if (name.empty() || name == "") {
        return std::nullopt;
    }
    auto e = event_repo_->findByName(name);
    if (!e.has_value()) {
        return std::nullopt;
    }

    return e;
}

std::optional<model::Event> EventService::CreateEvent(model::Event& e) {
    if (e.name.empty() || e.name == "") {
        return std::nullopt;
    }

    auto it = event_repo_->findByName(e.name);
    if (it.has_value()) {
        return std::nullopt;
    }

    event_repo_->save(e);
}

bool EventService::SetStatus(int event_id, model::EventStatus status){
    auto e = event_repo_->findById(event_id);
    if (!e.has_value()) {
        return false;
    }

    event_repo_->setStatusById(event_id, status);
    return true;
}