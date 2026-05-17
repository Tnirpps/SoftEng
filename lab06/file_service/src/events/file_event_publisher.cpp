#include "events/file_event_publisher.hpp"

#include <chrono>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/uuid4.hpp>

namespace Events {
namespace {

constexpr std::string_view kExchangeName = "madisk.events";
constexpr std::string_view kFileCreatedRoutingKey = "file.created";
constexpr std::string_view kFileUpdatedRoutingKey = "file.updated";
constexpr std::string_view kFileDeletedRoutingKey = "file.deleted";

std::string GenerateEventId() {
    return userver::utils::ToString(
        userver::utils::BoostUuidFromString(userver::utils::generators::GenerateUuid()));
}

std::string NowIsoString() {
    return userver::utils::datetime::Timestring(userver::utils::datetime::Now());
}

}  // namespace

FileEventPublisher::FileEventPublisher(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context),
      client_(context.FindComponent<userver::components::RabbitMQ>(
                  config["rabbit_name"].As<std::string>())
                  .GetClient()),
      exchange_(std::string{kExchangeName}) {
    const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds{2});
    auto admin_channel = client_->GetAdminChannel(deadline);
    admin_channel.DeclareExchange(exchange_, userver::urabbitmq::Exchange::Type::kDirect, deadline);
}

void FileEventPublisher::PublishFileCreated(const std::string& file_id,
                                            const std::string& owner_id,
                                            const std::optional<std::string>& directory_id,
                                            const std::string& name) const {
    Publish("FileCreated", std::string{kFileCreatedRoutingKey}, file_id, owner_id, directory_id, std::optional<std::string>{name});
}

void FileEventPublisher::PublishFileUpdated(const std::string& file_id,
                                            const std::string& owner_id,
                                            const std::optional<std::string>& directory_id,
                                            const std::string& name) const {
    Publish("FileUpdated", std::string{kFileUpdatedRoutingKey}, file_id, owner_id, directory_id, std::optional<std::string>{name});
}

void FileEventPublisher::PublishFileDeleted(const std::string& file_id,
                                            const std::string& owner_id,
                                            const std::optional<std::string>& directory_id) const {
    Publish("FileDeleted", std::string{kFileDeletedRoutingKey}, file_id, owner_id, directory_id, std::nullopt);
}

void FileEventPublisher::Publish(const std::string& event_type,
                                 const std::string& routing_key,
                                 const std::string& file_id,
                                 const std::string& owner_id,
                                 const std::optional<std::string>& directory_id,
                                 const std::optional<std::string>& name) const {
    userver::formats::json::ValueBuilder payload;
    payload["event_id"] = GenerateEventId();
    payload["event_type"] = event_type;
    payload["occurred_at"] = NowIsoString();
    payload["file_id"] = file_id;
    payload["owner_id"] = owner_id;

    if (directory_id.has_value()) {
        payload["directory_id"] = *directory_id;
    } else {
        payload["directory_id"] = nullptr;
    }

    if (name.has_value()) {
        payload["name"] = *name;
    }

    const userver::urabbitmq::Envelope envelope{
        userver::formats::json::ToString(payload.ExtractValue()),
        userver::urabbitmq::MessageType::kTransient,
        {},
        {},
        {},
    };

    client_->PublishReliable(
        exchange_,
        routing_key,
        envelope,
        userver::engine::Deadline::FromDuration(std::chrono::seconds{2}));
}

}  // namespace Events
