#include "App.h"
#include "LoginScreen.h"
#include "RegisterScreen.h"
#include "VisitScreen.h"
// #include "gui/screens/HomeScreen.h"
// #include "gui/screens/BillListScreen.h"
// #include "gui/screens/BillCreateScreen.h"
// #include "gui/screens/StatisticsScreen.h"
// #include "gui/screens/ProfileScreen.h"
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
    // router.Register(Route::Register, []() { return CreateRegisterScreen(); });
    // router.Register(Route::Home, []() { return CreateHomeScreen(); });
    // router.Register(Route::BillList, []() { return CreateBillListScreen(); });
    // router.Register(Route::BillCreate, []() { return CreateBillCreateScreen(); });
    // router.Register(Route::Statistics, []() { return CreateStatisticsScreen(); });
    // router.Register(Route::Profile, []() { return CreateProfileScreen(); });
}

void App::Run() {
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto& router = Router::Instance();
    
    auto renderer = ftxui::Renderer([&] {
        return router.GetCurrentScreen()->Render();
    });

    auto main_container = CatchEvent(renderer, [&](ftxui::Event e) {
        return router.GetCurrentScreen()->OnEvent(e);
    });

    router.SetOnRouteChange([&](Route) {
        screen.PostEvent(ftxui::Event::Custom);
    });
    
    screen.Loop(main_container);
}