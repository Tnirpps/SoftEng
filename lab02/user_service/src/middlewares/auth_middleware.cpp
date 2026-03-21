#include "auth_middleware.hpp"

#include <userver/formats/json/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/http/status_code.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include "auth/jwt_credentials.hpp"
#include "handlers/base_handler.hpp"
#include "schemas/openapi.hpp"

namespace {
void MakeErrorResponse(userver::server::http::HttpRequest &request,
                       Gen::openapi::ErrorResponse &&error_response) {
    auto &response = request.GetHttpResponse();
    response.SetStatus(static_cast<userver::http::StatusCode>(error_response.status));
    response.SetData(
        userver::formats::json::ToString(
            userver::formats::json::ValueBuilder{std::move(error_response)}.ExtractValue()));
}
} // namespace

namespace Middlewares {

AuthMiddleware::AuthMiddleware(const userver::server::handlers::HttpHandlerBase &handler,
                               const Auth::JwtCredentials &jwt_credentials)
    : handler_(handler)
    , jwt_credentials_(jwt_credentials) {
}

void AuthMiddleware::HandleRequest(userver::server::http::HttpRequest &request,
                                   userver::server::request::RequestContext &context) const {
    std::string auth_header = request.GetHeader("Authorization");
    if (auth_header.empty()) {
        MakeErrorResponse(request, Handlers::Error401("Authorization header was not provided"));
        return;
    }
    const std::string bearer_prefix = "Bearer ";
    if (auth_header.rfind(bearer_prefix, 0) != 0) {
        MakeErrorResponse(
            request,
            Handlers::Error401(
                "Invalid Authorization header format. Expected 'Bearer <token>'"));
        return;
    }

    std::string token = auth_header.substr(bearer_prefix.length());
    if (token.empty()) {
        MakeErrorResponse(request, Handlers::Error401("Token was not provided"));
        return;
    }

    try {
        auto decoded_token = jwt::decode<jwt::traits::nlohmann_json>(token);

        auto verifier = jwt::verify<jwt::traits::nlohmann_json>()
                            .allow_algorithm(jwt::algorithm::hs256{jwt_credentials_.GetSecret()})
                            .with_issuer(jwt_credentials_.GetIssuer());

        verifier.verify(decoded_token);
        auto subject = decoded_token.get_subject();
        LOG_INFO() << "JWT verified for user: " << subject;
        auto payload = userver::formats::json::FromString(decoded_token.get_payload());
        if ((!payload.HasMember("login") || !payload["login"].IsString()) || (!payload.HasMember("uuid") || !payload["uuid"].IsString())) {
            LOG_WARNING() << "Bad JWT payload for subject: " << subject;
            MakeErrorResponse(request, Handlers::Error401("Invalid token"));
            return;
        }
        context.SetUserData(payload);
    } catch (const jwt::error::token_verification_exception &e) {
        LOG_WARNING() << "Token verification failed: " << e.what();
        MakeErrorResponse(request, Handlers::Error401("Invalid token"));
        return;
    } catch (const std::exception &e) {
        LOG_WARNING() << "JWT error: " << e.what();
        MakeErrorResponse(request, Handlers::Error401("Invalid token"));
        return;
    }

    Next(request, context);
}

AuthMiddlewareFactory::AuthMiddlewareFactory(const userver::components::ComponentConfig &config,
                                             const userver::components::ComponentContext &context)
    : HttpMiddlewareFactoryBase(config, context)
    , jwt_credentials_(context.FindComponent<Auth::JwtCredentials>()) {
}

std::unique_ptr<userver::server::middlewares::HttpMiddlewareBase>
AuthMiddlewareFactory::Create(
    const userver::server::handlers::HttpHandlerBase &handler,
    userver::yaml_config::YamlConfig /*config*/
) const {
    return std::make_unique<AuthMiddleware>(handler, jwt_credentials_);
}

} // namespace Middlewares
