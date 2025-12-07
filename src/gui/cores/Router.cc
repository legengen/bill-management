#include "Router.h"

void Router::Register(Route route, ScreenFactory factory) {
    routes_[route] = factory;

    if (route == current_route_ && !current_screen_) {
        current_screen_ = factory();
    }
}

void Router::NavigateTo(Route route) {
    current_route_ = route;
    
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