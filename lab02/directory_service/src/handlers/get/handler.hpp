#pragma once

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>

#include "handlers/base_handler.hpp"
#include "schemas/openapi.hpp"

namespace Handlers {

class GetHandler final
    : public Handlers::TypedHandler<
          Gen::openapi::Directory,
          userver::server::http::HttpStatus::kOk> {
  public:
    static constexpr std::string_view kName = "handler-directory-get";

    using Response = TypedHandler::Response;

    GetHandler(const userver::components::ComponentConfig &config,
               const userver::components::ComponentContext &context);

    ResponseVariant HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                       userver::server::request::RequestContext &context) const override;
};

} // namespace Handlers
