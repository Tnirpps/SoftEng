#pragma once

#include <memory>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_status.hpp>
#include "auth/auth_repository.hpp"

#include "handlers/base_handler.hpp"
#include "schemas/openapi.hpp"

class RegisterHandler final
    : public Handlers::TypedJsonHandler<Gen::openapi::UserCreateRequest, Gen::openapi::UserCreateResponse,
                                        userver::server::http::HttpStatus::kCreated> {
  public:
    static constexpr std::string_view kName = "handler-user-registration";

    using Response = TypedJsonHandler::Response;
    using Request = TypedJsonHandler::Request;

    RegisterHandler(const userver::components::ComponentConfig &config,
                    const userver::components::ComponentContext &context);

    ResponseVariant HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                       const Gen::openapi::UserCreateRequest &body,
                                       userver::server::request::RequestContext &context) const override;

  private:
    std::shared_ptr<auth::IAuthRepository> auth_repository_;
};
