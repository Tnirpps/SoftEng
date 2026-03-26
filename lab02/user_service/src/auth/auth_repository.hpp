#pragma once

#include <memory>
#include <optional>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/schema.hpp>
#include <variant>

#include "models/user.hpp"

namespace Auth {

enum class AddUserError {
    UserAlreadyExists,
    ServerError
};

using AddUserResult = std::variant<Models::User, AddUserError>;
using CheckUserResult = std::optional<Models::User>;
using SearchUserResult = std::optional<Models::User>;

class IAuthRepository {
  public:
    virtual ~IAuthRepository() = default;

    virtual CheckUserResult CheckUser(const std::string &login, const std::string &password) = 0;
    virtual AddUserResult AddUser(const std::string &login, const std::string &password, const std::string &first_name, const std::string &last_name) = 0;
    virtual SearchUserResult SearchUserByPattern(const std::string &pattern) = 0;
    virtual void DeleteAllUsers() = 0;
};

class AuthComponent : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "auth-repository";

    AuthComponent(const userver::components::ComponentConfig &config,
                  const userver::components::ComponentContext &context);

    std::shared_ptr<IAuthRepository> GetRepository();

    static userver::yaml_config::Schema GetStaticConfigSchema();

  private:
    std::shared_ptr<IAuthRepository> repository_;
};

} // namespace Auth
