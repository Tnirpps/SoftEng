#include "handler.hpp"

LoginHandler::LoginHandler(const userver::components::ComponentConfig &config,
                           const userver::components::ComponentContext &context)
    : userver::server::handlers::HttpHandlerJsonBase(config, context) {
}

userver::formats::json::Value
LoginHandler::HandleRequestJsonThrow(const userver::server::http::HttpRequest & /*request*/,
                                     const userver::formats::json::Value & /*value*/,
                                     userver::server::request::RequestContext & /*context*/) const {
    return {};
}

// std::shared_ptr<auth::IAuthRepository> auth_repository_;
