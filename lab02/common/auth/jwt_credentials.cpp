#include "jwt_credentials.hpp"

#include <userver/logging/log.hpp>

namespace Auth {

JwtCredentials::JwtCredentials(const userver::components::ComponentConfig &config,
                               const userver::components::ComponentContext &context)
    : ComponentBase(config, context)
    , secret_(config["secret"].As<std::string>())
    , issuer_(config["issuer"].As<std::string>()) {
    LOG_INFO() << "JwtCredentials initialized";
}

const std::string &JwtCredentials::GetSecret() const {
    return secret_;
}

const std::string &JwtCredentials::GetIssuer() const {
    return issuer_;
}

userver::yaml_config::Schema JwtCredentials::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: JWT credentials component for storing secret key and issuer
additionalProperties: false
properties:
    secret:
        type: string
        description: JWT secret key for token generation and validation
    issuer:
        type: string
        description: JWT issuer claim
)");
}

} // namespace Auth
