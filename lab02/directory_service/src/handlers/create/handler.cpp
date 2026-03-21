#include "handler.hpp"

namespace Handlers {

CreateHandler::CreateHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context) {
}

CreateHandler::ResponseVariant
CreateHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                  const Gen::openapi::DirectoryCreateRequest & /*body*/,
                                  userver::server::request::RequestContext & /*context*/) const {
    // TODO: реализовать логику создания директории
    throw std::runtime_error("Not implemented");
}

} // namespace Handlers
