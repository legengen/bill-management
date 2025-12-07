#include "Router.h"

void Router::Register(Route route, ScreenFactory factory) {
    routes_[route] = factory;
}

void Router::NavigateTo(Route route) {
    current_route_ = route;
    
    if (on_route_change_) {
        on_route_change_(route);
    }
}

ftxui::Component Router::GetCurrentScreen() {
    auto it = routes_.find(current_route_);
    if (it != routes_.end()) {
        return it->second();
    }
    return ftxui::Container::Vertical({});
}