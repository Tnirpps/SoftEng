#pragma once

#include <string>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/yaml_config/schema.hpp>

namespace Auth {

/**
 * @brief Component for storing JWT credentials (secret key and issuer)
 *
 * Simple configuration component that holds JWT credentials
 * for token generation and validation.
 */
class JwtCredentials : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "jwt-credentials";

    JwtCredentials(const userver::components::ComponentConfig &config,
                   const userver::components::ComponentContext &context);

    /**
     * @brief Get the JWT secret key
     * @return JWT secret key string
     */
    const std::string &GetSecret() const;

    /**
     * @brief Get the JWT issuer
     * @return JWT issuer string
     */
    const std::string &GetIssuer() const;

    static userver::yaml_config::Schema GetStaticConfigSchema();

  private:
    std::string secret_;
    std::string issuer_;
};

} // namespace Auth
