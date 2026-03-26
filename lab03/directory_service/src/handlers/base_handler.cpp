#include "base_handler.hpp"

#include <userver/formats/json/value_builder.hpp>
#include <userver/utils/overloaded.hpp>

namespace Handlers {

Gen::openapi::ErrorResponse Error400(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kBadRequest, .extra = extra};
}

Gen::openapi::ErrorResponse Error401(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kUnauthorized, .extra = extra};
}

Gen::openapi::ErrorResponse Error403(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kForbidden, .extra = extra};
}

Gen::openapi::ErrorResponse Error404(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kNotFound, .extra = extra};
}

Gen::openapi::ErrorResponse Error409(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kConflict, .extra = extra};
}

Gen::openapi::ErrorResponse Error422(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kUnprocessableEntity, .extra = extra};
}

Gen::openapi::ErrorResponse Error429(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kTooManyRequests, .extra = extra};
}

Gen::openapi::ErrorResponse Error500(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kInternalServerError, .extra = extra};
}

Gen::openapi::ErrorResponse Error501(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kNotImplemented, .extra = extra};
}

Gen::openapi::ErrorResponse Error503(const std::string &message, const userver::formats::json::Value &extra) {
    return {.message = message, .status = userver::server::http::HttpStatus::kServiceUnavailable, .extra = extra};
}

} // namespace Handlers
