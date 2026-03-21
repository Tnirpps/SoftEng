#include "handler.hpp"

namespace Handlers {

FilesListHandler::FilesListHandler(const userver::components::ComponentConfig &config,
                                   const userver::components::ComponentContext &context)
    : TypedHandler(config, context) {
}

FilesListHandler::ResponseVariant
FilesListHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                     userver::server::request::RequestContext & /*context*/) const {
    // TODO: реализовать логику получения списка файлов в директории
    throw std::runtime_error("Not implemented");
}

} // namespace Handlers
