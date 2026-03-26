#pragma once

#include <memory>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_status.hpp>

#include "handlers/base_handler.hpp"
#include "repositories/directory_repository.hpp"
#include "schemas/openapi.hpp"

namespace Handlers {

class MoveHandler final
    : public Handlers::TypedJsonHandler<
          Gen::openapi::DirectoryMoveRequest,
          Gen::openapi::Directory,
          userver::server::http::HttpStatus::kOk> {
  public:
    static constexpr std::string_view kName = "handler-directory-move";

    using Response = TypedJsonHandler::Response;
    using Request = TypedJsonHandler::Request;

    MoveHandler(const userver::components::ComponentConfig &config,
                const userver::components::ComponentContext &context);

    ResponseVariant HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                       const Gen::openapi::DirectoryMoveRequest &body,
                                       userver::server::request::RequestContext &context) const override;

  private:
    std::shared_ptr<Repositories::IDirectoryRepository> directory_repository_;
};

} // namespace Handlers
