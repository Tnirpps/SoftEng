#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/concurrent/variable.hpp>
#include <variant>

#include "models/user.hpp"

namespace Auth {

/**
 * @brief Error types for AddUser operation
 */
enum class AddUserError {
    UserAlreadyExists, // Клиентская ошибка (409 Conflict)
    ServerError        // Серверная ошибка (500 Internal Server Error)
};

/**
 * @brief Result type for AddUser operation
 * Uses std::variant to represent either success (User) or error (AddUserError)
 */
using AddUserResult = std::variant<Models::User, AddUserError>;
using CheckUserResult = std::optional<Models::User>;

/**
 * @brief Abstract interface for authentication repository
 *
 * Defines the contract for user credential storage and verification.
 * This allows swapping In-Memory storage with PostgreSQL or Redis
 * without changing handler code.
 */
class IAuthRepository {
  public:
    virtual ~IAuthRepository() = default;

    /**
     * @brief Check if user credentials are valid
     * @param login User loginname/login
     * @param password User password
     * @return true if credentials are valid, false otherwise
     */
    virtual CheckUserResult CheckUser(const std::string &login, const std::string &password) = 0;

    /**
     * @brief Add a new user to the repository
     * @param login User loginname/login
     * @param password User password (should be hashed in production)
     * @return AddUserResult - User on success, AddUserError on failure
     */
    virtual AddUserResult AddUser(const std::string &login, const std::string &password) = 0;
};

/**
 * @brief Thread-safe in-memory implementation of IAuthRepository
 *
 * Uses userver::concurrent::Variable<std::map> for thread-safe access
 * to user credentials.
 */
class InMemoryAuthRepository : public IAuthRepository {
  public:
    InMemoryAuthRepository() = default;

    /**
     * @brief Initialize repository with pre-configured users
     * @param initial_users Map of login -> password pairs
     */
    explicit InMemoryAuthRepository(const std::map<std::string, std::string> &initial_users);

    CheckUserResult CheckUser(const std::string &login, const std::string &password) override;
    AddUserResult AddUser(const std::string &login, const std::string &password) override;

  private:
    userver::concurrent::Variable<std::map<std::string, std::string>> users_;
};

/**
 * @brief Component wrapper for AuthRepository
 *
 * Handles component lifecycle, configuration reading, and provides
 * access to the repository implementation via ComponentContext.
 */
class AuthComponent : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "auth-repository";

    AuthComponent(const userver::components::ComponentConfig &config,
                  const userver::components::ComponentContext &context);

    /**
     * @brief Get reference to the underlying repository
     * @return Reference to InMemoryAuthRepository instance
     */
    std::shared_ptr<InMemoryAuthRepository> GetRepository();

    static userver::yaml_config::Schema GetStaticConfigSchema();

  private:
    std::shared_ptr<InMemoryAuthRepository> repository_;
};

} // namespace Auth
