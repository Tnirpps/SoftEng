#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/rabbitmq.hpp>

namespace Events {

class FileEventPublisher final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "file-event-publisher";

    FileEventPublisher(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

    void PublishFileCreated(const std::string& file_id,
                            const std::string& owner_id,
                            const std::optional<std::string>& directory_id,
                            const std::string& name) const;

    void PublishFileUpdated(const std::string& file_id,
                            const std::string& owner_id,
                            const std::optional<std::string>& directory_id,
                            const std::string& name) const;

    void PublishFileDeleted(const std::string& file_id,
                            const std::string& owner_id,
                            const std::optional<std::string>& directory_id) const;

  private:
    void Publish(const std::string& event_type,
                 const std::string& routing_key,
                 const std::string& file_id,
                 const std::string& owner_id,
                 const std::optional<std::string>& directory_id,
                 const std::optional<std::string>& name) const;

    std::shared_ptr<userver::urabbitmq::Client> client_;
    userver::urabbitmq::Exchange exchange_;
};

}  // namespace Events
