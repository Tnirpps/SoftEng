#include "handler.hpp"

#include <userver/formats/json/value.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/http/http_status.hpp>

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

    bool found = auth_repository_->SearchUserByPattern(pattern);

    return Response{.found = found};
}

} // namespace Handlers
