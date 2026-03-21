#include "handler.hpp"

namespace Handlers {

ListHandler::ListHandler(const userver::components::ComponentConfig &config,
                         const userver::components::ComponentContext &context)
    : TypedHandler(config, context) {
}

ListHandler::ResponseVariant
ListHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                userver::server::request::RequestContext & /*context*/) const {
    // TODO: реализовать логику получения списка директорий
    throw std::runtime_error("Not implemented");
}

} // namespace Handlers
