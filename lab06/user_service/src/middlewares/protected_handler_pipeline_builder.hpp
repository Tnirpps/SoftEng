#pragma once

#include <userver/components/component.hpp>
#include <userver/server/middlewares/configuration.hpp>

#include "auth_middleware.hpp"

namespace Middlewares {

class ProtectedHandlerPipelineBuilder final
    : public userver::server::middlewares::HandlerPipelineBuilder {
  public:
    static constexpr std::string_view kName{"protected-handler-pipeline-builder"};

    using HandlerPipelineBuilder::HandlerPipelineBuilder;

    userver::server::middlewares::MiddlewaresList BuildPipeline(
        userver::server::middlewares::MiddlewaresList server_middleware_pipeline) const override {
        auto pipeline = std::move(server_middleware_pipeline);
        pipeline.emplace_back(Middlewares::AuthMiddlewareFactory::kName);
        return pipeline;
    }
};

} // namespace Middlewares
