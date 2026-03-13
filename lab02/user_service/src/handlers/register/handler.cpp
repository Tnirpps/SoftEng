#include "handler.hpp"
#include <userver/formats/json/value.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/datetime.hpp>
#include "handlers/base_handler.hpp"

RegisterHandler::RegisterHandler(const userver::components::ComponentConfig &config,
                                 const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , auth_repository_(context.FindComponent<auth::AuthComponent>().GetRepository()) {
}

RegisterHandler::ResponseVariant
RegisterHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                    const Gen::openapi::UserCreateRequest &body,
                                    userver::server::request::RequestContext & /*context*/) const {
    if (body.login.empty() || body.password.empty()) {
        return Handlers::Error400("Missing required fields");
    }

    if (auth_repository_->CheckUser(body.login, body.password)) {
        return Handlers::Error400("User already exists");
    }

    // Добавляем пользователя в репозиторий
    if (!auth_repository_->AddUser(body.login, body.password)) {
        return Handlers::Error409("User already exists");
    }

    return Response{.login = body.login,
                    .first_name = body.first_name,
                    .last_name = body.last_name,
                    .created_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()}};
}
