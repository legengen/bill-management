#include "StatisticsService.h"

std::vector<model::Bill> StatisticsService::QueryByTimeInOrder(
    model::Timestamp from, model::Timestamp to) {
    
    if (from > to) {
        return {};
    }
    
    return bill_repository_->queryByTimeInOrder(from, to);
}

std::vector<model::Bill> StatisticsService::QueryByTimeAndEventInOrder(
    model::Timestamp from, model::Timestamp to) {
    
    if (from > to) {
        return {};
    }
    
    return bill_repository_->queryByTimeAndEventInOrder(from, to);
}