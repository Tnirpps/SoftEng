#include "events/file_events_consumer.hpp"

#include <chrono>
#include <string>

#include <userver/components/component_context.hpp>
#include <userver/formats/json/exception.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/logging/log.hpp>

namespace Events {
namespace {

const userver::urabbitmq::Exchange kExchange{"madisk.events"};
const userver::urabbitmq::Queue kQueue{"directory_service.file_events"};
constexpr std::string_view kFileCreatedRoutingKey = "file.created";
constexpr std::string_view kFileUpdatedRoutingKey = "file.updated";
constexpr std::string_view kFileDeletedRoutingKey = "file.deleted";

}  // namespace

FileEventsConsumer::FileEventsConsumer(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : userver::urabbitmq::ConsumerComponentBase(config, context),
      cache_(context.FindComponent<Cache::DirectoryCacheComponent>().GetCache()) {
    auto client = context.FindComponent<userver::components::RabbitMQ>(
                     config["rabbit_name"].As<std::string>())
                     .GetClient();

    const auto deadline = userver::engine::Deadline::FromDuration(std::chrono::seconds{2});
    auto admin_channel = client->GetAdminChannel(deadline);
    admin_channel.DeclareExchange(kExchange, userver::urabbitmq::Exchange::Type::kDirect, deadline);
    admin_channel.DeclareQueue(kQueue, deadline);
    admin_channel.BindQueue(kExchange, kQueue, std::string{kFileCreatedRoutingKey}, deadline);
    admin_channel.BindQueue(kExchange, kQueue, std::string{kFileUpdatedRoutingKey}, deadline);
    admin_channel.BindQueue(kExchange, kQueue, std::string{kFileDeletedRoutingKey}, deadline);
}

void FileEventsConsumer::Process(std::string message) {
    try {
        const auto event = userver::formats::json::FromString(message);

        if (!event.HasMember("owner_id") || !event.HasMember("directory_id") ||
            event["directory_id"].IsNull()) {
            LOG_INFO() << "Skip file event without directory_id";
            return;
        }

        const auto owner_id = event["owner_id"].As<std::string>();
        const auto directory_id = event["directory_id"].As<std::string>();

        cache_.InvalidateDirectoryFiles(owner_id, directory_id);

        LOG_INFO() << "Processed file event for directory files read model"
                   << ", owner_id=" << owner_id
                   << ", directory_id=" << directory_id;
    } catch (const userver::formats::json::Exception& ex) {
        LOG_WARNING() << "Failed to parse file event from RabbitMQ: " << ex.what();
    }
}

}  // namespace Events
