#pragma once
#include <string>
#include <functional>
#include <map>
#include <stack>
#include <memory>
#include <ftxui/component/component.hpp>

enum class Route {
    Visit,
    Login,
    Register,
    Home,
    ChangePassword,
    BillManage,
    BillQuery,
    BillStats,
    BillCreate,
    EventManage,
    UserManage,
    Profile,
    QueryByEvent,
};

class Router {
public:
    using ScreenFactory = std::function<ftxui::Component()>;
    
    static Router& Instance() {
        static Router instance;
        return instance;
    }

    void Register(Route route, ScreenFactory factory);
    void NavigateTo(Route route);
    void Exit();

    ftxui::Component GetCurrentScreen();
    Route GetCurrentRoute() const { return current_route_; }

    void SetOnRouteChange(std::function<void(Route)> callback) {
        on_route_change_ = callback;
    }

    void SetExitCallback(std::function<void()> callback) {
        on_exit_ = callback;
    }
    
private:
    Router() = default;
    
    std::map<Route, ScreenFactory> routes_;
    Route current_route_ = Route::Visit;
    ftxui::Component current_screen_;
    std::function<void(Route)> on_route_change_;
    std::function<void()> on_exit_;
    ftxui::Component current_component_;
};