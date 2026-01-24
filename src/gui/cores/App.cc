#include "App.h"
#include "Dialog.h"
#include "LoginScreen.h"
#include "RegisterScreen.h"
#include "VisitScreen.h"
#include "HomeScreen.h"
#include "ChangePasswordScreen.h"
#include "UserManageScreen.h"
#include "EventManageScreen.h"
#include "BillManageScreen.h"
#include "BillCreateScreen.h"
#include "BillQueryScreen.h"
// #include "StatisticsScreen.h"
#include "ProfileScreen.h"
#include "QueryByEventScreen.h"
#include <ftxui/component/screen_interactive.hpp>

App* App::instance_ = nullptr;

App::App(std::shared_ptr<AuthService> auth_service,
         std::shared_ptr<BillService> bill_service,
         std::shared_ptr<EventService> event_service,
         std::shared_ptr<UserService> user_service,
         std::shared_ptr<StatisticsService> stats_service)
    : auth_service_(auth_service)
    , bill_service_(bill_service)
    , event_service_(event_service)
    , user_service_(user_service)
    , stats_service_(stats_service) {
    instance_ = this;
    RegisterScreens();
}

void App::RegisterScreens() {
    auto& router = Router::Instance();
    
    router.Register(Route::Login, []() { return CreateLoginScreen(); });
    router.Register(Route::Visit, []() { return CreateVisitScreen(); });
    router.Register(Route::Register, []() { return CreateRegisterScreen(); });
    router.Register(Route::Home, []() { return CreateHomeScreen(); });
    router.Register(Route::ChangePassword, []() { return CreateChangePasswordScreen(); });
    router.Register(Route::UserManage, []() { return CreateUserManageScreen(); });
    router.Register(Route::EventManage, []() { return CreateEventManageScreen(); });
    router.Register(Route::BillManage, []() { return CreateBillManageScreen(); });
    router.Register(Route::BillCreate, []() { return CreateBillCreateScreen(); });
    router.Register(Route::BillQuery, []() { return CreateBillQueryScreen(); });
    router.Register(Route::Profile, []() { return CreateProfileScreen(); });
    router.Register(Route::QueryByEvent, []() { return CreateQueryByEventScreen(); });
}

void App::Run() {
    auto screen = ftxui::ScreenInteractive::FitComponent();
    auto& router = Router::Instance();

    router.SetOnRouteChange([&](Route) {
        screen.PostEvent(ftxui::Event::Custom);
    });

    router.SetExitCallback([&]() {
        screen.Exit();
    });

    auto renderer = ftxui::Renderer([&] {
        return router.GetCurrentScreen()->Render();
    });

    auto main_with_events = CatchEvent(renderer, [&](ftxui::Event e) {
        return router.GetCurrentScreen()->OnEvent(e);
    });

    auto main_component = WithDialog(main_with_events);
    
    screen.Loop(main_component);
}