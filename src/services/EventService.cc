#include "EventService.h"

std::optional<model::Event> EventService::QueryByName(const std::string& name) {
    if (name.empty() || name == "") {
        return std::nullopt;
    }
    auto e = event_repository_->findByName(name);
    if (!e.has_value()) {
        return std::nullopt;
    }

    return e;
}

std::optional<model::Event> EventService::QueryById(int event_id) {
    if (event_id <= 0) {
        return std::nullopt;
    }

    auto e = event_repository_->findById(event_id);
    if (!e.has_value()) {
        return std::nullopt;
    }

    return e;
}

std::optional<model::Event> EventService::CreateEvent(model::Event& e) {
    if (e.name.empty() || e.name == "") {
        return std::nullopt;
    }

    auto it = event_repository_->findByName(e.name);
    if (it.has_value()) {
        return std::nullopt;
    }

    event_repository_->save(e);
    return e;
}

bool EventService::SetStatus(int event_id, model::EventStatus status){
    if (event_id <= 0) {
        return false;
    }

    auto e = event_repository_->findById(event_id);
    if (!e.has_value()) {
        return false;
    }

    if (!event_repository_->setStatusById(event_id, status)) {
        return false;
    }
    return true;
}