#pragma once
#include <irepositories.h>

class BillService {
public:
    explicit BillService(
        std::shared_ptr<repo::IBillRepository> bill_repo, 
        std::shared_ptr<repo::IAnnotationRepository> anno_repo,
        std::shared_ptr<repo::IUserRepository> user_repository
        ):
        bill_repository_(bill_repo), 
        annotation_repository_(anno_repo), 
        user_repository_(user_repository) {}
    std::optional<model::Bill> CreateBill(int owner_id, model::Bill data);
    std::vector<model::Bill> QueryByTime(int owner_id, model::Timestamp from, model::Timestamp to);
    std::vector<model::Bill> queryByEvent(int owner_id, int event_id);
    std::vector<model::Bill> queryByPhone(std::string phone);
    void editBill(int bill_id, model::Bill updates);
    void deleteBill(int bill_id);
    void annotateBill(int bill_id, model::Annotation a);
    int GetNextBillId(int owner_id);

    std::string GetLastError() const { return last_error_; }
private:
    std::shared_ptr<repo::IBillRepository> bill_repository_;
    std::shared_ptr<repo::IAnnotationRepository> annotation_repository_;
    std::shared_ptr<repo::IUserRepository> user_repository_;
    std::string last_error_;
}; 


