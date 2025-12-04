#include <irepositories.h>

class StatisticsService {
public:
    explicit StatisticsService(std::shared_ptr<repo::IBillRepository> bill_repo):
        bill_repository_(bill_repo) {}
    std::vector<model::Bill> QueryByTimeInOrder(model::Timestamp from, model::Timestamp to);
    std::vector<model::Bill> QueryByTimeAndEventInOrder(model::Timestamp from, model::Timestamp to);
    
private:
    std::shared_ptr<repo::IBillRepository> bill_repository_;
};

