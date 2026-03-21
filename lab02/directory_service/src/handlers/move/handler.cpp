#include "handler.hpp"

namespace Handlers {

MoveHandler::MoveHandler(const userver::components::ComponentConfig &config,
                         const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context) {
}

MoveHandler::ResponseVariant
MoveHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                const Gen::openapi::DirectoryMoveRequest & /*body*/,
                                userver::server::request::RequestContext & /*context*/) const {
    // TODO: реализовать логику перемещения директории
    throw std::runtime_error("Not implemented");
}

} // namespace Handlers
