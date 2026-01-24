#include "BillCreateScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "Dialog.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <vector>

using namespace ftxui;

struct BillCreateState {
    std::vector<std::string> event_names;
    std::vector<int> event_ids;
    int event_selected = 0;
    std::string money_str;
    std::string description;
    int next_bill_id = 0;
    
    void LoadEvents() {
        auto& event_service = App::Instance().GetEventService();
        auto events = event_service.GetAllEvents();
        
        event_names.clear();
        event_ids.clear();
        
        for (const auto& event : events) {
            if (event.status == model::EventStatus::Available) {
                event_names.push_back(event.name);
                event_ids.push_back(event.id);
            }
        }
        
        if (event_names.empty()) {
            event_names.push_back("(无可用事件)");
            event_ids.push_back(-1);
        }
    }

    void LoadNextBillId() {
        auto& session = Session::Instance();
        auto& bill_service = App::Instance().GetBillService();
        next_bill_id = bill_service.GetNextBillId();
    }
    
    int GetSelectedEventId() const {
        if (event_selected >= 0 && event_selected < static_cast<int>(event_ids.size())) {
            return event_ids[event_selected];
        }
        return -1;
    }
    
    bool ParseMoney(double& out) const {
        if (money_str.empty()) return false;
        try {
            out = std::stod(money_str);
            return out > 0;
        } catch (...) {
            return false;
        }
    }
};

ftxui::Component CreateBillCreateScreen() {
    auto state = std::make_shared<BillCreateState>();
    state->LoadEvents();
    state->LoadNextBillId();

    auto event_dropdown = Dropdown(&state->event_names, &state->event_selected);
    auto money_input = Input(&state->money_str, "输入金额");
    auto description_input = Input(&state->description, "输入描述");

    auto confirm_button = Button("确认", [state] {
        int event_id = state->GetSelectedEventId();
        if (event_id < 0) {
            DialogManager::Instance().ShowError("请选择有效的事件类型");
            return;
        }
        
        double amount = 0.0;
        if (!state->ParseMoney(amount)) {
            DialogManager::Instance().ShowError("请输入有效的金额（大于0）");
            return;
        }

        auto& session = Session::Instance();
        int owner_id = session.GetUserId();
        
        model::Bill bill_data;
        bill_data.owner_id = owner_id;
        bill_data.event_id = event_id;
        bill_data.amount = amount;
        bill_data.description = state->description;
        bill_data.created_at = model::Now();

        auto& bill_service = App::Instance().GetBillService();
        auto result = bill_service.CreateBill(owner_id, bill_data);
        
        if (result.has_value()) {
            auto user = session.GetCurrentUser();
            if (user.has_value()) {
                session.UpdateBalance(user->balance + bill_data.amount);
            }
            
            DialogManager::Instance().ShowSuccess(
                "创建成功！账单号: " + std::to_string(state->next_bill_id)
            );
            Router::Instance().NavigateTo(Route::Home);
        } else {
            DialogManager::Instance().ShowError(bill_service.GetLastError());
        }
    });

    auto return_button = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });

    auto container = Container::Vertical({
        event_dropdown,
        money_input,
        description_input,
        Container::Horizontal({confirm_button, return_button}),
    });

    return Renderer(container, [=] {
        int col_w = 25;

        auto welcome_text = text("新建手账") | bold | center;

        const int LABEL_WIDTH = 10;

        const int BUTTON_WIDTH = 12;

        const int INPUT_WIDTH = 15;
        // 显示下一个账单编号
        auto LT = hbox({
            text("账单号:") | center | size(WIDTH, EQUAL, LABEL_WIDTH),
            filler() | size(WIDTH, EQUAL, 2),
            text(std::to_string(state->next_bill_id)) | center,
        });

        auto RT = hbox({
            text("事件类型:") | center | size(WIDTH, EQUAL, LABEL_WIDTH),
            filler(),
            event_dropdown->Render() | size(WIDTH, EQUAL, INPUT_WIDTH) | center,
        });

        auto LB = hbox({
            text("金额:") | center | size(WIDTH, EQUAL, LABEL_WIDTH),
            filler(),
            money_input->Render() | border | size(WIDTH, EQUAL, INPUT_WIDTH) | center,
        });

        auto RB = hbox({
            text("描述:") | center | size(WIDTH, EQUAL, LABEL_WIDTH),
            filler(),
            description_input->Render() | border | size(WIDTH, EQUAL, INPUT_WIDTH) | center,
        });

        auto row1 = hbox({
            LT | size(WIDTH, EQUAL, col_w),
            filler(),
            RT | size(WIDTH, EQUAL, col_w),
        });

        auto row2 = hbox({
            LB | size(WIDTH, EQUAL, col_w),
            filler(),
            RB | size(WIDTH, EQUAL, col_w),
        });

        auto buttons = hbox({
            filler(),
            confirm_button->Render() | size(WIDTH, EQUAL, 12),
            filler(),
            return_button->Render() | size(WIDTH, EQUAL, 12),
            filler(),
        });

        auto content = vbox({
            welcome_text,
            filler() | size(HEIGHT, EQUAL, 1),
            RenderUserInfoBar(PageType::Profile),
            text(""),
            row1,
            text(""),
            row2,
            text(""),
            buttons,
        });

        return vbox({
            filler() | size(HEIGHT, EQUAL, 1),
            hbox({
                filler() | size(WIDTH, EQUAL, 2),
                content | flex,
                filler() | size(WIDTH, EQUAL, 2),
            }),
            filler() | size(HEIGHT, EQUAL, 1),
        }) | border;
    });
}