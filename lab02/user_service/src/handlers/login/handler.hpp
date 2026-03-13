#pragma once

#include <memory>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include "auth/auth_repository.hpp"

class LoginHandler final : public userver::server::handlers::HttpHandlerJsonBase {
  public:
    static constexpr std::string_view kName = "handler-user-login";

    LoginHandler(const userver::components::ComponentConfig &config,
                 const userver::components::ComponentContext &context);

    userver::formats::json::Value
    HandleRequestJsonThrow(const userver::server::http::HttpRequest &request,
                           const userver::formats::json::Value &value,
                           userver::server::request::RequestContext &context) const override;

  private:
    std::shared_ptr<auth::IAuthRepository> auth_repository_;
};
