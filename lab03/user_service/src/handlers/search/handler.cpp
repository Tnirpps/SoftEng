#include "handler.hpp"

#include <userver/formats/json/value.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/utils/boost_uuid4.hpp>

#include "auth/auth_repository.hpp"
#include "handlers/base_handler.hpp"

namespace Handlers {

SearchHandler::SearchHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
    , auth_repository_(context.FindComponent<Auth::AuthComponent>().GetRepository()) {
}

SearchHandler::ResponseVariant
SearchHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                  userver::server::request::RequestContext & /*context*/) const {

    // Получаем query-параметр 'pattern'
    std::string pattern = request.GetArg("pattern");

    if (pattern.empty()) {
        return Handlers::Error400("Missing required query parameter: pattern");
    }
    if (pattern.length() > 20) {
        return Handlers::Error400("Pattern length must not exceed 20 characters");
    }

    auto user_opt = auth_repository_->SearchUserByPattern(pattern);

    if (user_opt.has_value()) {
        const auto &user = user_opt.value();
        return Response{
            .found = true,
            .user = Response::User{
                .uuid = userver::utils::BoostUuidFromString(user.uuid),
                .login = user.login,
                .first_name = user.first_name,
                .last_name = user.last_name,
                .created_at = user.created_at}};
    } else {
        return Response{.found = false};
    }
}

} // namespace Handlers
