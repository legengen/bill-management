#include "Router.h"

void Router::Register(Route route, ScreenFactory factory) {
    routes_[route] = factory;
}

void Router::NavigateTo(Route route) {
    if (route == current_route_) {
        return;
    }

    auto it = routes_.find(route);
    if (it == routes_.end()) {
        return;
    }

    current_route_ = route;
    current_screen_ = it->second();
    
    if (on_route_change_) {
        on_route_change_(route);
    }
}

ftxui::Component Router::GetCurrentScreen() {
    if (!current_screen_) {
        auto it = routes_.find(current_route_);
        if (it != routes_.end()) {
            current_screen_ = it->second();
        } else {
            current_screen_ = ftxui::Container::Vertical({});
        }
    }
    return current_screen_;
}