#pragma once

#include <string_view>

#include <userver/components/component_context.hpp>
#include <userver/rabbitmq.hpp>

#include "cache/directory_cache.hpp"

namespace Events {

class FileEventsConsumer final : public userver::urabbitmq::ConsumerComponentBase {
  public:
    static constexpr std::string_view kName = "file-events-consumer";

    FileEventsConsumer(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

  protected:
    void Process(std::string message) override;

  private:
    const Cache::DirectoryCache& cache_;
};

}  // namespace Events
