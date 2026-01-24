#include "EventManageScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
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

enum class EventDialogType {
    None,
    Create,
    EditStatus
};

struct EventManageState {
    std::vector<model::Event> events;
    int current_page = 0;
    int page_size = 5;
    int selected_row = 0;
    
    // 查询相关
    std::string search_name;
    bool is_filtered = false;
    
    // 弹窗相关
    EventDialogType dialog_type = EventDialogType::None;
    std::string new_event_name;
    int next_event_id = 1;
    int editing_event_index = -1;
    
    void LoadAllEvents() {
        auto& event_service = App::Instance().GetEventService();
        events = event_service.GetAllEvents();
        is_filtered = false;
        current_page = 0;
        selected_row = 0;
        UpdateNextEventId();
    }
    
    void UpdateNextEventId() {
        next_event_id = 1;
        for (const auto& e : events) {
            if (e.id >= next_event_id) {
                next_event_id = e.id + 1;
            }
        }
    }
    
    void SearchByName() {
        if (search_name.empty()) {
            LoadAllEvents();
            return;
        }
        
        auto& event_service = App::Instance().GetEventService();
        auto event = event_service.QueryByName(search_name);
        
        events.clear();
        if (event.has_value()) {
            events.push_back(*event);
        }
        
        is_filtered = true;
        current_page = 0;
        selected_row = 0;
        
        if (events.empty()) {
            DialogManager::Instance().ShowInfo("未找到该事项");
        }
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
    
    void OpenCreateDialog() {
        new_event_name.clear();
        UpdateNextEventId();
        dialog_type = EventDialogType::Create;
    }
    
    void OpenEditStatusDialog() {
        auto* event = GetSelectedEvent();
        if (event) {
            editing_event_index = current_page * page_size + selected_row;
            dialog_type = EventDialogType::EditStatus;
        }
    }
    
    bool CreateEvent() {
        if (new_event_name.empty()) {
            return false;
        }
        
        auto& event_service = App::Instance().GetEventService();
        
        // 检查名称是否已存在
        auto existing = event_service.QueryByName(new_event_name);
        if (existing.has_value()) {
            return false;
        }
        
        model::Event new_event;
        new_event.id = 0;  // 自动分配
        new_event.name = new_event_name;
        new_event.status = 1;  // 正常状态
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
    
    void CloseDialog() {
        dialog_type = EventDialogType::None;
        new_event_name.clear();
        editing_event_index = -1;
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

ftxui::Component CreateEventManageScreen() {
    auto state = std::make_shared<EventManageState>();
    state->LoadAllEvents();
    
    // 搜索组件
    auto search_input = Input(&state->search_name, "输入事项名称");
    
    auto search_btn = Button("查询", [state] {
        state->SearchByName();
    });
    
    auto show_all_btn = Button("显示全部", [state] {
        state->search_name.clear();
        state->LoadAllEvents();
    });
    
    auto create_btn = Button("新建事项", [state] {
        state->OpenCreateDialog();
    });
    
    // 翻页按钮
    auto prev_btn = Button("◀ 上一页", [state] {
        state->PrevPage();
    });
    
    auto next_btn = Button("下一页 ▶", [state] {
        state->NextPage();
    });
    
    // 返回按钮
    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::Home);
    });
    
    // 新建弹窗组件
    auto new_event_input = Input(&state->new_event_name, "事项名称");
    
    auto create_confirm_btn = Button("确认", [state] {
        if (state->new_event_name.empty()) {
            DialogManager::Instance().ShowError("请输入事项名称");
            return;
        }
        if (state->CreateEvent()) {
            DialogManager::Instance().ShowSuccess("创建成功");
            state->CloseDialog();
        } else {
            DialogManager::Instance().ShowError("创建失败，名称可能已存在");
        }
    });
    
    auto create_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    // 状态修改弹窗组件
    auto set_normal_btn = Button("设为正常", [state] {
        if (state->SetEventStatus(model::EventStatus::Available)) {
            DialogManager::Instance().ShowSuccess("状态已修改为正常");
            state->CloseDialog();
        } else {
            DialogManager::Instance().ShowError("修改失败");
        }
    });
    
    auto set_frozen_btn = Button("设为冻结", [state] {
        if (state->SetEventStatus(model::EventStatus::Frozen)) {
            DialogManager::Instance().ShowSuccess("状态已修改为冻结");
            state->CloseDialog();
        } else {
            DialogManager::Instance().ShowError("修改失败");
        }
    });
    
    auto status_cancel_btn = Button("取消", [state] {
        state->CloseDialog();
    });
    
    // 主容器
    auto main_container = Container::Vertical({
        Container::Horizontal({search_input, search_btn, show_all_btn, create_btn}),
        Container::Horizontal({prev_btn, next_btn}),
        return_btn,
    });
    
    // 新建弹窗容器
    auto create_dialog_container = Container::Vertical({
        new_event_input,
        Container::Horizontal({create_confirm_btn, create_cancel_btn}),
    });
    
    // 状态修改弹窗容器
    auto status_dialog_container = Container::Horizontal({
        set_normal_btn,
        set_frozen_btn,
        status_cancel_btn,
    });
    
    // Tab 切换
    auto tab_index = std::make_shared<int>(0);
    auto container = Container::Tab({
        main_container,
        create_dialog_container,
        status_dialog_container,
    }, tab_index.get());
    
    // 键盘事件处理
    container = CatchEvent(container, [state](Event event) {
        if (state->dialog_type != EventDialogType::None) {
            return false;
        }
        
        if (event == Event::ArrowUp) {
            state->SelectPrevRow();
            return true;
        }
        if (event == Event::ArrowDown) {
            state->SelectNextRow();
            return true;
        }
        if (event == Event::ArrowLeft) {
            state->PrevPage();
            return true;
        }
        if (event == Event::ArrowRight) {
            state->NextPage();
            return true;
        }
        if (event == Event::Return) {
            if (state->GetSelectedEvent()) {
                state->OpenEditStatusDialog();
                return true;
            }
        }
        return false;
    });

    return Renderer(container, [=] {
        // 设置 tab 索引
        switch (state->dialog_type) {
            case EventDialogType::None: *tab_index = 0; break;
            case EventDialogType::Create: *tab_index = 1; break;
            case EventDialogType::EditStatus: *tab_index = 2; break;
        }
        
        // 新建事项弹窗
        if (state->dialog_type == EventDialogType::Create) {
            return vbox({
                filler(),
                hbox({
                    filler(),
                    vbox({
                        text("新建事项") | bold | center,
                        separator(),
                        text(""),
                        hbox({
                            text("事项编号: "),
                            text(std::to_string(state->next_event_id)) | bold,
                        }) | center,
                        text(""),
                        hbox({
                            text("事项名称: "),
                            new_event_input->Render() | size(WIDTH, EQUAL, 20) | border,
                        }) | center,
                        text(""),
                        separator(),
                        hbox({
                            filler(),
                            create_confirm_btn->Render() | size(WIDTH, EQUAL, 10),
                            text("  "),
                            create_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                            filler(),
                        }),
                    }) | border | size(WIDTH, EQUAL, 40),
                    filler(),
                }),
                filler(),
            }) | border;
        }
        
        // 状态修改弹窗
        if (state->dialog_type == EventDialogType::EditStatus) {
            auto* event = (state->editing_event_index >= 0 && 
                          state->editing_event_index < static_cast<int>(state->events.size())) 
                          ? &state->events[state->editing_event_index] : nullptr;
            
            std::string event_info = event ? 
                ("事项: " + event->name + " (ID: " + std::to_string(event->id) + ")") : "未知事项";
            std::string current_status = event ? 
                ("当前状态: " + state->GetStatusText(event->status)) : "";
            
            return vbox({
                filler(),
                hbox({
                    filler(),
                    vbox({
                        text("修改事项状态") | bold | center,
                        separator(),
                        text(""),
                        text(event_info) | center,
                        text(current_status) | center | dim,
                        text(""),
                        separator(),
                        hbox({
                            filler(),
                            set_normal_btn->Render() | size(WIDTH, EQUAL, 12),
                            text(" "),
                            set_frozen_btn->Render() | size(WIDTH, EQUAL, 12),
                            text(" "),
                            status_cancel_btn->Render() | size(WIDTH, EQUAL, 10),
                            filler(),
                        }),
                    }) | border | size(WIDTH, EQUAL, 45),
                    filler(),
                }),
                filler(),
            }) | border;
        }
        
        // 主页面
        auto title = text("事项管理") | bold | center;
        auto user_info = RenderUserInfoBar(PageType::Home);
        
        Element filter_status;
        if (state->is_filtered) {
            filter_status = hbox({
                text("查询: ") | bold,
                text(state->search_name) | color(Color::Green),
                text("  找到 " + std::to_string(state->events.size()) + " 条记录"),
            }) | center;
        } else {
            filter_status = text("显示全部事项，共 " + std::to_string(state->events.size()) + " 条记录") | center | dim;
        }
        
        auto search_area = hbox({
            text("事项名称: ") | center,
            search_input->Render() | size(WIDTH, EQUAL, 15) | border,
            text(" "),
            search_btn->Render() | size(WIDTH, EQUAL, 8),
            text(" "),
            show_all_btn->Render() | size(WIDTH, EQUAL, 10),
            text(" "),
            create_btn->Render() | size(WIDTH, EQUAL, 10),
        }) | center;
        
        // 构建表格
        std::vector<std::vector<std::string>> table_data;
        table_data.push_back({"", "编号", "事项名称", "创建时间", "状态"});
        
        auto current_events = state->GetCurrentPageEvents();
        for (int i = 0; i < static_cast<int>(current_events.size()); i++) {
            const auto& event = current_events[i];
            std::string selector = (i == state->selected_row) ? ">" : " ";
            table_data.push_back({
                selector,
                std::to_string(event.id),
                event.name,
                state->FormatTime(event.created_at),
                state->GetStatusText(event.status)
            });
        }
        
        if (current_events.empty()) {
            table_data.push_back({"", "", "暂无事项数据", "", ""});
        }
        
        auto table = Table(table_data);
        table.SelectAll().Border(LIGHT);
        table.SelectRow(0).Decorate(bold);
        table.SelectColumn(0).DecorateCells(size(WIDTH, EQUAL, 3));
        table.SelectColumn(1).DecorateCells(size(WIDTH, EQUAL, 8));
        table.SelectColumn(2).DecorateCells(size(WIDTH, EQUAL, 15));
        table.SelectColumn(3).DecorateCells(size(WIDTH, EQUAL, 18));
        table.SelectColumn(4).DecorateCells(size(WIDTH, EQUAL, 8));
        table.SelectAll().DecorateCells(center);
        
        // 高亮选中行
        if (state->selected_row >= 0 && state->selected_row < static_cast<int>(current_events.size())) {
            table.SelectRow(state->selected_row + 1).Decorate(bgcolor(Color::Blue));
        }
        
        auto table_element = table.Render();
        
        auto page_info = text("第 " + std::to_string(state->current_page + 1) + 
                              " / " + std::to_string(state->GetTotalPages()) + " 页  " +
                              "(↑↓选择, ←→翻页, Enter修改状态)") | center;
        
        auto page_buttons = hbox({
            filler(),
            prev_btn->Render() | size(WIDTH, EQUAL, 12),
            text("  "),
            next_btn->Render() | size(WIDTH, EQUAL, 12),
            filler(),
        });
        
        auto bottom_buttons = hbox({
            filler(),
            return_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
        });
        
        auto content = vbox({
            title,
            user_info,
            separator(),
            text(""),
            search_area,
            filter_status,
            text(""),
            table_element | center,
            text(""),
            page_info,
            page_buttons,
            text(""),
            separator(),
            bottom_buttons,
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