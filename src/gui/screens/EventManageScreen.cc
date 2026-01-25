#include "EventManageScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "PageLayout.h"
#include "Dialog.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <memory>
#include <vector>
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace ftxui;

struct EventManageState {
    std::vector<model::Event> events;
    int current_page = 0;
    int page_size = 3;
    int selected_row = 0;
    
    std::string search_name;
    std::string new_event_name;
    int editing_event_index = -1;
    int next_event_id = 1;
    
    void LoadAllEvents() {
        auto& event_service = App::Instance().GetEventService();
        events = event_service.GetAllEvents();
        current_page = 0;
        selected_row = 0;
        UpdateNextEventId();
    }

    void UpdateNextEventId() {
        if (events.empty()) {
            next_event_id = 1;
        } else {
            int max_id = 0;
            for (const auto& event : events) {
                if (event.id > max_id) {
                    max_id = event.id;
                }
            }
            next_event_id = max_id + 1;
        }
    }
    
    bool SearchByName() {
        if (search_name.empty()) {
            LoadAllEvents();
            return true;
        }
        
        auto& event_service = App::Instance().GetEventService();
        auto event = event_service.QueryByName(search_name);
        
        events.clear();
        if (event.has_value()) {
            events.push_back(*event);
            current_page = 0;
            selected_row = 0;
            return true;
        }
        return false;
    }
    
    int GetTotalPages() const {
        if (events.empty()) return 1;
        return (events.size() + page_size - 1) / page_size;
    }
    
    std::vector<model::Event> GetCurrentPageEvents() const {
        std::vector<model::Event> result;
        int start = current_page * page_size;
        int end = std::min(start + page_size, static_cast<int>(events.size()));
        
        for (int i = start; i < end; i++) {
            result.push_back(events[i]);
        }
        return result;
    }
    
    void NextPage() {
        if (current_page < GetTotalPages() - 1) {
            current_page++;
            selected_row = 0;
        }
    }
    
    void PrevPage() {
        if (current_page > 0) {
            current_page--;
            selected_row = 0;
        }
    }
    
    void SelectNextRow() {
        auto page_events = GetCurrentPageEvents();
        if (selected_row < static_cast<int>(page_events.size()) - 1) {
            selected_row++;
        }
    }
    
    void SelectPrevRow() {
        if (selected_row > 0) {
            selected_row--;
        }
    }
    
    model::Event* GetSelectedEvent() {
        int global_index = current_page * page_size + selected_row;
        if (global_index >= 0 && global_index < static_cast<int>(events.size())) {
            return &events[global_index];
        }
        return nullptr;
    }
    
    void OpenEditDialog() {
        auto* event = GetSelectedEvent();
        if (event) {
            editing_event_index = current_page * page_size + selected_row;
        }
    }
    
    bool CreateEvent() {
        if (new_event_name.empty()) {
            return false;
        }
        
        auto& event_service = App::Instance().GetEventService();
        
        auto existing = event_service.QueryByName(new_event_name);
        if (existing.has_value()) {
            return false;
        }
        
        model::Event new_event;
        new_event.id = 0;
        new_event.name = new_event_name;
        new_event.status = model::EventStatus::Available;
        new_event.created_at = std::time(nullptr);
        
        auto result = event_service.CreateEvent(new_event);
        if (result.has_value()) {
            LoadAllEvents();
            return true;
        }
        return false;
    }
    
    bool SetEventStatus(int status) {
        if (editing_event_index < 0 || editing_event_index >= static_cast<int>(events.size())) {
            return false;
        }
        
        auto& event = events[editing_event_index];
        auto& event_service = App::Instance().GetEventService();
        
        if (event_service.SetStatus(event.id, status)) {
            event.status = status;
            return true;
        }
        return false;
    }
    
    std::string FormatTime(model::Timestamp ts) const {
        std::time_t time = static_cast<std::time_t>(ts);
        std::tm* tm = std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M");
        return oss.str();
    }
    
    std::string GetStatusText(int status) const {
        return status == model::EventStatus::Available ? "正常" : "冻结";
    }
};

enum class EventDialogType {
    None,
    Create,
    EditStatus,
};

ftxui::Component CreateEventManageScreen() {
    auto state = std::make_shared<EventManageState>();
    state->LoadAllEvents();
    
    auto current_dialog = std::make_shared<EventDialogType>(EventDialogType::None);
    auto tab_index = std::make_shared<int>(0);

    // ==================== 搜索输入框 ====================
    auto search_input = Input(&state->search_name, "输入事项名称");

    auto search_btn = Button("搜索", [state] {
        if (state->SearchByName()) {
            if (!state->search_name.empty()) {
                DialogManager::Instance().ShowSuccess("找到 " + std::to_string(state->events.size()) + " 个事项");
            }
        } else {
            DialogManager::Instance().ShowError("未找到该事项");
        }
    });

    // ==================== 新建事项弹窗 ====================
    auto new_event_input = Input(&state->new_event_name, "输入事项名称");

    auto create_confirm_btn = Button("确认", [state, current_dialog] {
        if (state->new_event_name.empty()) {
            DialogManager::Instance().ShowError("请输入事项名称");
            return;
        }
        if (state->CreateEvent()) {
            *current_dialog = EventDialogType::None;
            state->new_event_name.clear();
            DialogManager::Instance().ShowSuccess("创建成功");
        } else {
            DialogManager::Instance().ShowError("创建失败，名称可能已存在");
        }
    });

    auto create_cancel_btn = Button("取消", [state, current_dialog] {
        *current_dialog = EventDialogType::None;
        state->new_event_name.clear();
    });

    auto create_dialog_container = Container::Vertical({
        new_event_input,
        Container::Horizontal({create_confirm_btn, create_cancel_btn}),
    });

    // ==================== 修改状态弹窗 ====================
    auto set_normal_btn = Button("设为正常", [state, current_dialog] {
        if (state->SetEventStatus(model::EventStatus::Available)) {
            *current_dialog = EventDialogType::None;
            DialogManager::Instance().ShowSuccess("状态已修改为正常");
        } else {
            DialogManager::Instance().ShowError("修改失败");
        }
    });

    auto set_frozen_btn = Button("设为冻结", [state, current_dialog] {
        if (state->SetEventStatus(model::EventStatus::Frozen)) {
            *current_dialog = EventDialogType::None;
            DialogManager::Instance().ShowSuccess("状态已修改为冻结");
        } else {
            DialogManager::Instance().ShowError("修改失败");
        }
    });

    auto status_cancel_btn = Button("取消", [current_dialog] {
        *current_dialog = EventDialogType::None;
    });

    auto status_dialog_container = Container::Horizontal({
        set_normal_btn,
        set_frozen_btn,
        status_cancel_btn,
    });

    // ==================== 主页面按钮 ====================
    auto create_btn = Button("新建事项", [state, current_dialog] {
        state->new_event_name.clear();
        *current_dialog = EventDialogType::Create;
    });

    auto edit_status_btn = Button("修改状态", [state, current_dialog] {
        if (state->GetSelectedEvent()) {
            state->OpenEditDialog();
            *current_dialog = EventDialogType::EditStatus;
        } else {
            DialogManager::Instance().ShowError("请先选择事项");
        }
    });

    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });

    // ==================== 主容器（使用 Horizontal 避免上下键切换按钮） ====================
    auto main_container = Container::Horizontal({
        search_input, 
        search_btn, 
        create_btn,
        edit_status_btn,
        return_btn,
    });

    // ==================== Tab 容器 ====================
    auto container = Container::Tab({
        main_container,           // 0: 主页面
        create_dialog_container,  // 1: 新建弹窗
        status_dialog_container,  // 2: 修改状态弹窗
    }, tab_index.get());

    LayoutConfig config;
    config.button_width = 12;
    config.content_padding = 4;
    config.outer_padding = 2;
    config.content_height = 15;

    const int COL_ID = 8;
    const int COL_NAME = 16;
    const int COL_STATUS = 10;
    const int COL_TIME = 18;

    return Renderer(container, [=]() {
        switch (*current_dialog) {
            case EventDialogType::None:       *tab_index = 0; break;
            case EventDialogType::Create:     *tab_index = 1; break;
            case EventDialogType::EditStatus: *tab_index = 2; break;
        }
        
        auto user_info = RenderUserInfoBar();
        
        // ========== 搜索栏和操作按钮 ==========
        auto search_row = hbox({
            text("事项名称: ") | vcenter,
            search_input->Render() | size(WIDTH, EQUAL, 20) | border,
            text(" "),
            search_btn->Render() | size(WIDTH, EQUAL, 10),
            text(" "),
            create_btn->Render() | size(WIDTH, EQUAL, 12),
            text(" "),
            edit_status_btn->Render() | size(WIDTH, EQUAL, 12),
        }) | center;
        
        // ========== 构建表格 ==========
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"编号", "事项名称", "状态", "创建时间"});
        
        auto page_events = state->GetCurrentPageEvents();
        for (const auto& event : page_events) {
            table_data.push_back({
                std::to_string(event.id),
                event.name,
                state->GetStatusText(event.status),
                state->FormatTime(event.created_at),
            });
        }
        
        while (table_data.size() < static_cast<size_t>(state->page_size + 1)) {
            table_data.push_back({"", "", "", ""});
        }
        
        auto table = Table(table_data);
        
        table.SelectAll().SeparatorVertical(LIGHT);
        table.SelectAll().SeparatorHorizontal(LIGHT);
        table.SelectAll().Border(LIGHT);
        
        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, COL_ID));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, COL_NAME));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, COL_STATUS));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, COL_TIME));
        
        table.SelectAll().DecorateCells(center);
        
        table.SelectRow(0).Decorate(bold);
        table.SelectRow(0).DecorateCells(bgcolor(Color::Yellow));
        
        if (state->selected_row >= 0 && state->selected_row < static_cast<int>(page_events.size())) {
            table.SelectRow(state->selected_row + 1).DecorateCells(bgcolor(Color::GrayDark));
        }
        
        auto table_element = table.Render();
        
        auto page_info = hbox({
            text("第 " + std::to_string(state->current_page + 1) + 
                 "/" + std::to_string(state->GetTotalPages()) + " 页"),
            text("  "),
            text("← → 翻页"),
        }) | center;
        
        auto content_area = vbox({
            search_row,
            text(""),
            table_element | center,
        });
        
        auto button_area = hbox({
            return_btn->Render() | size(WIDTH, EQUAL, config.button_width),
            filler(),
            page_info,
        });
        
        auto main_page = CreatePageLayout(user_info, content_area, button_area, config);
        
        // ==================== 弹窗渲染 ====================
        if (*current_dialog == EventDialogType::Create) {
            auto dialog_box = vbox({
                hbox({
                    text("事项ID: "),
                    text(std::to_string(state->next_event_id)) | bold | color(Color::Cyan) | vcenter,
                }) | center,
                text(""),
                hbox({
                    text("事项名称: ") | center,
                    new_event_input->Render() | size(WIDTH, EQUAL, 20) | border,
                }) | center,
                hbox({
                    filler(),
                    create_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                    text("  "),
                    create_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        if (*current_dialog == EventDialogType::EditStatus) {
            auto* event = state->GetSelectedEvent();
            std::string event_name = event ? event->name : "";
            std::string event_id = event ? std::to_string(event->id) : "";
            std::string current_status = event ? state->GetStatusText(event->status) : "";
            Color status_color = (event && event->status == model::EventStatus::Available) 
                                 ? Color::Green : Color::Red;
            
            auto dialog_box = vbox({
                hbox({
                    text("事项: "),
                    text(event_name),
                }) | center,
                text(""),
                hbox({
                    text("当前状态: ") | vcenter,
                    text(current_status) | bold | color(status_color) | vcenter,
                }) | center,
                hbox({
                    filler(),
                    set_normal_btn->Render() | size(WIDTH, EQUAL, 12),
                    text(" "),
                    set_frozen_btn->Render() | size(WIDTH, EQUAL, 12),
                    text(" "),
                    status_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                    filler(),
                }),
            }) | border | clear_under | center;
            
            return dbox({
                main_page | dim,
                dialog_box,
            });
        }
        
        return main_page;
    }) | CatchEvent([state, current_dialog](Event event) {
        // 弹窗打开时不处理
        if (*current_dialog != EventDialogType::None) {
            return false;
        }
        
        // 上下键选择表格行
        if (event == Event::ArrowUp) {
            state->SelectPrevRow();
            return true;
        }
        if (event == Event::ArrowDown) {
            state->SelectNextRow();
            return true;
        }
        
        // 左右键翻页
        if (event == Event::ArrowLeft) {
            state->PrevPage();
            return true;
        }
        if (event == Event::ArrowRight) {
            state->NextPage();
            return true;
        }
        
        return false;
    });
}