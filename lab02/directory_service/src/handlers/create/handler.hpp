#pragma once

#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_status.hpp>

#include "handlers/base_handler.hpp"
#include "schemas/openapi.hpp"

namespace Handlers {

class CreateHandler final
    : public Handlers::TypedJsonHandler<
          Gen::openapi::DirectoryCreateRequest,
          Gen::openapi::Directory,
          userver::server::http::HttpStatus::kCreated> {
  public:
    static constexpr std::string_view kName = "handler-directory-create";

    using Response = TypedJsonHandler::Response;
    using Request = TypedJsonHandler::Request;

    CreateHandler(const userver::components::ComponentConfig &config,
                  const userver::components::ComponentContext &context);

    ResponseVariant HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                       const Gen::openapi::DirectoryCreateRequest &body,
                                       userver::server::request::RequestContext &context) const override;
};

} // namespace Handlers
