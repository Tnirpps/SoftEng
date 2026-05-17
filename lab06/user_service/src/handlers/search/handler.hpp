#pragma once

#include <memory>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>

#include "auth/auth_repository.hpp"
#include "handlers/base_handler.hpp"
#include "schemas/openapi.hpp"

namespace Handlers {

class SearchHandler final
    : public Handlers::TypedHandler<Gen::openapi::UserSearchResponse,
                                    userver::server::http::HttpStatus::kOk> {
  public:
    static constexpr std::string_view kName = "handler-user-search";

    using Response = TypedHandler::Response;

    SearchHandler(const userver::components::ComponentConfig &config,
                  const userver::components::ComponentContext &context);

    ResponseVariant HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                       userver::server::request::RequestContext &context) const override;

  private:
    std::shared_ptr<Auth::IAuthRepository> auth_repository_;
};

} // namespace Handlers
