#include "BillStatsScreen.h"
#include "App.h"
#include "Router.h"
#include "Session.h"
#include "UserInfoBar.h"
#include "Dialog.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cmath>

using namespace ftxui;

struct UserStats {
    std::string phone;
    double total_amount = 0.0;
    int bill_count = 0;
};

struct BillStatsState {
    std::vector<UserStats> user_stats;
    double max_amount = 0.0;
    
    // 时间筛选
    std::string start_year;
    std::string start_month;
    std::string start_day;
    std::string end_year;
    std::string end_month;
    std::string end_day;
    
    bool is_filtered = false;
    std::string filter_desc;
    
    void LoadAllStats() {
        auto& bill_service = App::Instance().GetBillService();
        auto all_bills = bill_service.queryByPhone("");
        CalculateStats(all_bills);
        is_filtered = false;
        filter_desc.clear();
    }
    
    void LoadStatsByTimeRange() {
        if (start_year.empty() || start_month.empty() || start_day.empty() ||
            end_year.empty() || end_month.empty() || end_day.empty()) {
            DialogManager::Instance().ShowError("请输入完整的开始和结束日期");
            return;
        }
        
        try {
            int sy = std::stoi(start_year);
            int sm = std::stoi(start_month);
            int sd = std::stoi(start_day);
            int ey = std::stoi(end_year);
            int em = std::stoi(end_month);
            int ed = std::stoi(end_day);
            
            std::tm start_tm = {};
            start_tm.tm_year = sy - 1900;
            start_tm.tm_mon = sm - 1;
            start_tm.tm_mday = sd;
            start_tm.tm_hour = 0;
            start_tm.tm_min = 0;
            start_tm.tm_sec = 0;
            std::time_t start_time = std::mktime(&start_tm);
            
            std::tm end_tm = {};
            end_tm.tm_year = ey - 1900;
            end_tm.tm_mon = em - 1;
            end_tm.tm_mday = ed;
            end_tm.tm_hour = 23;
            end_tm.tm_min = 59;
            end_tm.tm_sec = 59;
            std::time_t end_time = std::mktime(&end_tm);
            
            if (start_time > end_time) {
                DialogManager::Instance().ShowError("开始日期不能晚于结束日期");
                return;
            }
            
            auto& bill_service = App::Instance().GetBillService();
            auto bills = bill_service.QueryByTime(0, start_time, end_time);
            
            CalculateStats(bills);
            
            is_filtered = true;
            filter_desc = start_year + "-" + start_month + "-" + start_day + 
                         " 至 " + end_year + "-" + end_month + "-" + end_day;
            
            if (bills.empty()) {
                DialogManager::Instance().ShowInfo("该时间段内无账单记录");
            }
        } catch (...) {
            DialogManager::Instance().ShowError("日期格式错误");
        }
    }
    
    void CalculateStats(const std::vector<model::Bill>& bills) {
        auto& user_service = App::Instance().GetUserService();
        
        std::map<int, UserStats> stats_map;
        
        for (const auto& bill : bills) {
            auto& stats = stats_map[bill.owner_id];
            
            if (stats.phone.empty()) {
                auto user = user_service.GetUser(bill.owner_id);
                stats.phone = user.has_value() ? user->phone : "未知";
            }
            
            stats.total_amount += std::abs(bill.amount);
            stats.bill_count++;
        }
        
        user_stats.clear();
        for (const auto& [user_id, stats] : stats_map) {
            user_stats.push_back(stats);
        }
        
        std::sort(user_stats.begin(), user_stats.end(), 
            [](const UserStats& a, const UserStats& b) {
                return a.total_amount > b.total_amount;
            });
        
        max_amount = 0.0;
        for (const auto& stats : user_stats) {
            max_amount = std::max(max_amount, stats.total_amount);
        }
        if (max_amount == 0.0) max_amount = 1.0;
    }
    
    void ClearFilter() {
        start_year.clear();
        start_month.clear();
        start_day.clear();
        end_year.clear();
        end_month.clear();
        end_day.clear();
        LoadAllStats();
    }
    
    std::string FormatAmount(double amount) const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << amount;
        return oss.str();
    }
};

ftxui::Component CreateBillStatsScreen() {
    auto state = std::make_shared<BillStatsState>();
    state->LoadAllStats();
    
    auto start_year_input = Input(&state->start_year, "年");
    auto start_month_input = Input(&state->start_month, "月");
    auto start_day_input = Input(&state->start_day, "日");
    auto end_year_input = Input(&state->end_year, "年");
    auto end_month_input = Input(&state->end_month, "月");
    auto end_day_input = Input(&state->end_day, "日");
    
    auto filter_btn = Button("筛选", [state] {
        state->LoadStatsByTimeRange();
    });
    
    auto clear_btn = Button("清除", [state] {
        state->ClearFilter();
    });
    
    auto return_btn = Button("返回", [] {
        Router::Instance().NavigateTo(Route::BillManage);
    });
    
    auto container = Container::Vertical({
        Container::Horizontal({
            start_year_input, start_month_input, start_day_input,
            end_year_input, end_month_input, end_day_input,
        }),
        Container::Horizontal({filter_btn, clear_btn, return_btn}),
    });

    return Renderer(container, [=] {
        auto title = text("账单统计") | bold | center;
        auto user_info = RenderUserInfoBar(PageType::Home);
        
        Element filter_status;
        if (state->is_filtered) {
            filter_status = hbox({
                text("筛选: ") | bold,
                text(state->filter_desc) | color(Color::Green),
            }) | center;
        } else {
            filter_status = text("显示全部账单统计") | center | dim;
        }
        
        // 柱形图区域
        const int BAR_MAX_WIDTH = 40;
        
        Elements chart_rows;
        
        if (state->user_stats.empty()) {
            chart_rows.push_back(text("暂无统计数据") | center | dim);
        } else {
            for (const auto& stats : state->user_stats) {
                int bar_width = static_cast<int>((stats.total_amount / state->max_amount) * BAR_MAX_WIDTH);
                bar_width = std::max(1, std::min(bar_width, BAR_MAX_WIDTH));
                
                std::string bar_str(bar_width, ' ');
                
                auto row = hbox({
                    text(stats.phone) | size(WIDTH, EQUAL, 14),
                    text(" "),
                    text(bar_str) | bgcolor(Color::White),
                    text(" "),
                    text(state->FormatAmount(stats.total_amount)),
                });
                
                chart_rows.push_back(row);
                chart_rows.push_back(text(""));
            }
        }
        
        auto chart = vbox(chart_rows);
        
        // 时间筛选区域
        auto time_filter = hbox({
            text("开始: ") | vcenter,
            start_year_input->Render() | size(WIDTH, EQUAL, 6) | border,
            text("-") | vcenter,
            start_month_input->Render() | size(WIDTH, EQUAL, 4) | border,
            text("-") | vcenter,
            start_day_input->Render() | size(WIDTH, EQUAL, 4) | border,
            text("  ") | vcenter,
            text("结束: ") | vcenter,
            end_year_input->Render() | size(WIDTH, EQUAL, 6) | border,
            text("-") | vcenter,
            end_month_input->Render() | size(WIDTH, EQUAL, 4) | border,
            text("-") | vcenter,
            end_day_input->Render() | size(WIDTH, EQUAL, 4) | border,
        }) | center;
        
        auto bottom_buttons = hbox({
            filler(),
            filter_btn->Render() | size(WIDTH, EQUAL, 10),
            text("  "),
            clear_btn->Render() | size(WIDTH, EQUAL, 10),
            text("  "),
            return_btn->Render() | size(WIDTH, EQUAL, 10),
            filler(),
        });
        
        auto content = vbox({
            title,
            user_info,
            separator(),
            text(""),
            filter_status,
            text(""),
            chart | center,
            text(""),
            time_filter,
            text(""),
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