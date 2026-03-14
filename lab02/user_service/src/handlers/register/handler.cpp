#include "handler.hpp"
#include <userver/formats/json/value.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/datetime.hpp>
#include "auth/auth_repository.hpp"
#include "handlers/base_handler.hpp"
#include "models/user.hpp"

namespace Handlers {

RegisterHandler::RegisterHandler(const userver::components::ComponentConfig &config,
                                 const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , auth_repository_(context.FindComponent<Auth::AuthComponent>().GetRepository()) {
}

RegisterHandler::ResponseVariant
RegisterHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                    const Gen::openapi::UserCreateRequest &body,
                                    userver::server::request::RequestContext & /*context*/) const {

    auto result = auth_repository_->AddUser(body.login, body.password);

    if (auto *error = std::get_if<Auth::AddUserError>(&result)) {
        switch (*error) {
        case Auth::AddUserError::UserAlreadyExists:
            return Handlers::Error409("User already exists");
        case Auth::AddUserError::ServerError:
            return Handlers::Error500("Internal server error");
        }
    }

    const auto &user = std::get<Models::User>(result);

    // TODO: store first_name and last_name in the database
    return Response{
        .login = user.login,
        .first_name = body.first_name,
        .last_name = body.last_name,
        .created_at = user.created_at};
}

} // namespace Handlers
