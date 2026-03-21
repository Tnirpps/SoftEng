#include "handler.hpp"

namespace Handlers {

GetHandler::GetHandler(const userver::components::ComponentConfig &config,
                       const userver::components::ComponentContext &context)
    : TypedHandler(config, context) {
}

GetHandler::ResponseVariant
GetHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                               userver::server::request::RequestContext & /*context*/) const {
    // TODO: реализовать логику получения информации о директории
    throw std::runtime_error("Not implemented");
}

} // namespace Handlers
