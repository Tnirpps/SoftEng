#include "handler.hpp"

#include <userver/formats/json/value.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#include "auth/auth_repository.hpp"
#include "auth/jwt_credentials.hpp"
#include "handlers/base_handler.hpp"
#include "models/user.hpp"

namespace Handlers {

LoginHandler::LoginHandler(const userver::components::ComponentConfig &config,
                           const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , auth_repository_(context.FindComponent<Auth::AuthComponent>().GetRepository())
    , jwt_credentials_(context.FindComponent<Auth::JwtCredentials>()) {
}

LoginHandler::ResponseVariant
LoginHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                 const Gen::openapi::UserLoginRequest &body,
                                 userver::server::request::RequestContext & /*context*/) const {

    auto result = auth_repository_->CheckUser(body.login, body.password);

    if (!result) {
        return Handlers::Error401("Invalid credentials");
    }

    const auto &user = *result;

    const auto now = userver::utils::datetime::Now();
    const auto exp = now + std::chrono::hours(24);

    auto token = jwt::create<jwt::traits::nlohmann_json>()
                     .set_issuer(jwt_credentials_.GetIssuer())
                     .set_type("JWT")
                     .set_subject(user.login)
                     .set_issued_at(now)
                     .set_expires_at(exp)
                     .sign(jwt::algorithm::hs256{jwt_credentials_.GetSecret()});

    LOG_INFO() << "[ABOBA] Geberated token: " << token;

    return Response{
        .login = user.login,
        .token = token,
    };
}

} // namespace Handlers
