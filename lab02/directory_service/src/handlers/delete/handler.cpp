#include "handler.hpp"

namespace Handlers {

DeleteHandler::DeleteHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedHandler(config, context) {
}

DeleteHandler::ResponseVariant
DeleteHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                  userver::server::request::RequestContext & /*context*/) const {
    // TODO: реализовать логику удаления директории
    throw std::runtime_error("Not implemented");
}

} // namespace Handlers
