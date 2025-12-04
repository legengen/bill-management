#include <irepositories.h>

class EventService{
public:
    std::optional<model::Event> QueryByName(const std::string& name);
    std::optional<model::Event> CreateEvent(model::Event& e);
    bool SetStatus(int event_id, model::EventStatus status);
private:
    std::shared_ptr<repo::IEventRepository> event_repo_;
};

