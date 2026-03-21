#include "handler.hpp"

namespace Handlers {

UpdateHandler::UpdateHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context) {
}

UpdateHandler::ResponseVariant
UpdateHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                  const Gen::openapi::DirectoryUpdateRequest & /*body*/,
                                  userver::server::request::RequestContext & /*context*/) const {
    // TODO: реализовать логику обновления директории
    throw std::runtime_error("Not implemented");
}

} // namespace Handlers
