#pragma once

#include <exception>
#include <string>
#include <userver/formats/json/value.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/server/http/http_request.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/server/request/request_context.hpp>
#include <userver/utils/overloaded.hpp>
#include <variant>

#include "schemas/openapi.hpp"

namespace Handlers {

/**
 * @brief 400 Bad Request - Invalid request data
 */
Gen::openapi::ErrorResponse Error400(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 401 Unauthorized - Authentication required
 */
Gen::openapi::ErrorResponse Error401(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 403 Forbidden - Access denied
 */
Gen::openapi::ErrorResponse Error403(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 404 Not Found - Resource not found
 */
Gen::openapi::ErrorResponse Error404(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 409 Conflict - Resource already exists
 */
Gen::openapi::ErrorResponse Error409(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 422 Unprocessable Entity - Validation error
 */
Gen::openapi::ErrorResponse Error422(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 429 Too Many Requests - Rate limit exceeded
 */
Gen::openapi::ErrorResponse Error429(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 500 Internal Server Error - Server error
 */
Gen::openapi::ErrorResponse Error500(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 501 Not Implemented - Method not implemented
 */
Gen::openapi::ErrorResponse Error501(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief 503 Service Unavailable - Service temporarily unavailable
 */
Gen::openapi::ErrorResponse Error503(const std::string &message, const userver::formats::json::Value &extra = {});

/**
 * @brief CRTP-based base handler for typed JSON request/response processing
 */
template <typename RequestType, typename ResponseType,
          userver::server::http::HttpStatus SuccessStatus = userver::server::http::HttpStatus::kOk>
class TypedJsonHandler : public userver::server::handlers::HttpHandlerJsonBase {

  public:
    using ResponseVariant = std::variant<ResponseType, Gen::openapi::ErrorResponse>;
    using Response = ResponseType;
    using Request = RequestType;

    // Forward constructors to base class
    using HttpHandlerJsonBase::HttpHandlerJsonBase;

    userver::formats::json::Value
    HandleRequestJsonThrow(const userver::server::http::HttpRequest &request,
                           const userver::formats::json::Value &value,
                           userver::server::request::RequestContext &context) const override {
        Request request_body;
        try {
            request_body = value.As<Request>();
        } catch (const std::exception &e) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            return userver::formats::json::ValueBuilder{Error400(e.what())}.ExtractValue();
        }

        ResponseVariant result = HandleTypedRequest(request, request_body, context);

        return userver::utils::Visit(
            std::move(result),
            [&request](ResponseType response) {
                request.SetResponseStatus(SuccessStatus);
                return userver::formats::json::ValueBuilder{response}.ExtractValue();
            },
            [&request](Gen::openapi::ErrorResponse error) {
                request.SetResponseStatus(static_cast<userver::server::http::HttpStatus>(error.status));
                return userver::formats::json::ValueBuilder{error}.ExtractValue();
            });
    }

  protected:
    virtual ResponseVariant HandleTypedRequest(const userver::server::http::HttpRequest &request, const Request &body,
                                               userver::server::request::RequestContext &context) const = 0;

    virtual ~TypedJsonHandler() = default;
};

} // namespace Handlers
