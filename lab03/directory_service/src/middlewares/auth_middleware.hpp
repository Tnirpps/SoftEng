#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/middlewares/http_middleware_base.hpp>
#include <userver/server/request/request_context.hpp>
#include <userver/yaml_config/yaml_config.hpp>

#include "auth/jwt_credentials.hpp"

namespace Middlewares {

class AuthMiddleware final : public userver::server::middlewares::HttpMiddlewareBase {
  public:
    static constexpr std::string_view kName{"auth-middleware"};

    AuthMiddleware(const userver::server::handlers::HttpHandlerBase &handler,
                   const Auth::JwtCredentials &jwt_credentials);

  private:
    void HandleRequest(userver::server::http::HttpRequest &request,
                       userver::server::request::RequestContext &context) const override;

    const userver::server::handlers::HttpHandlerBase &handler_;
    const Auth::JwtCredentials &jwt_credentials_;
};

class AuthMiddlewareFactory final : public userver::server::middlewares::HttpMiddlewareFactoryBase {
  public:
    static constexpr std::string_view kName{"auth-middleware"};

    AuthMiddlewareFactory(const userver::components::ComponentConfig &config,
                          const userver::components::ComponentContext &context);

  private:
    std::unique_ptr<userver::server::middlewares::HttpMiddlewareBase> Create(
        const userver::server::handlers::HttpHandlerBase &handler,
        userver::yaml_config::YamlConfig config) const override;

    const Auth::JwtCredentials &jwt_credentials_;
};

} // namespace Middlewares
