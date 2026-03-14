#include "auth_repository.hpp"

#include <memory>
#include <optional>
#include <regex>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/testsuite/tasks.hpp>
#include <userver/testsuite/testpoint.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/yaml_config/schema.hpp>

namespace Auth {

// ==================== InMemoryAuthRepository ====================

InMemoryAuthRepository::InMemoryAuthRepository(const std::map<std::string, std::string> &initial_users)
    : users_(initial_users) {
}

CheckUserResult InMemoryAuthRepository::CheckUser(const std::string &login, const std::string &password) {
    // Use Lock() for read-only access - efficient O(log N) lookup
    auto data_ptr = users_.Lock();
    auto it = data_ptr->find(login);
    if (it == data_ptr->end() || it->second != password) {
        return std::nullopt;
    }
    return Models::User{
        .login = login,
        .first_name = "",
        .last_name = "",
        .created_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()}};
}

AddUserResult InMemoryAuthRepository::AddUser(const std::string &login, const std::string &password) {
    // Use Lock() for thread-safe write access
    auto data_ptr = users_.Lock();
    auto [it, inserted] = data_ptr->emplace(login, password);

    if (inserted) {
        // User was added successfully - return the User object
        return Models::User{
            .login = login,
            .first_name = "",
            .last_name = "",
            .created_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()}};
    } else {
        // User already exists - return error
        return AddUserError::UserAlreadyExists;
    }
}

bool InMemoryAuthRepository::SearchUserByPattern(const std::string &pattern) {
    auto data_ptr = users_.Lock();

    // Simple pattern matching: % matches any sequence, _ matches single char
    // Convert SQL-like pattern to regex
    std::string regex_pattern = "^";
    for (char c : pattern) {
        if (c == '%') {
            regex_pattern += ".*";
        } else if (c == '_') {
            regex_pattern += ".";
        } else {
            regex_pattern += std::regex_replace(std::string(1, c), std::regex(R"([\^$.|?*+(){}\\])"), R"(\$&)");
        }
    }
    regex_pattern += "$";

    std::regex re(regex_pattern, std::regex::icase);

    for (const auto &[login, _] : *data_ptr) {
        if (std::regex_match(login, re)) {
            return true;
        }
    }
    return false;
}

void InMemoryAuthRepository::DeleteAllUsers() {
    auto data_ptr = users_.Lock();
    data_ptr->clear();
}

// ==================== AuthComponent ====================

AuthComponent::AuthComponent(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : ComponentBase(config, context)
    , repository_(std::make_shared<InMemoryAuthRepository>()) {
    // Read optional initial-users configuration
    if (config.HasMember("initial-users")) {
        const auto &users_array = config["initial-users"];
        for (const auto &user_entry : users_array) {
            const std::string login = user_entry["login"].As<std::string>();
            const std::string pass = user_entry["pass"].As<std::string>();
            repository_->AddUser(login, pass);
        }
    }

    // 1. Находим компонент поддержки тестов
    auto &testsuite_support = context.FindComponent<userver::components::TestsuiteSupport>();
    auto &tasks = testsuite_support.GetTestsuiteTasks();
    tasks.RegisterTask("delete-all-users", [repository = this->repository_]() {
        repository->DeleteAllUsers();
    });

    LOG_INFO() << "AuthComponent initialized";
}

std::shared_ptr<InMemoryAuthRepository> AuthComponent::GetRepository() {
    return repository_;
}

userver::yaml_config::Schema AuthComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: Auth repository component
additionalProperties: false
properties:
    initial-users:
        type: array
        description: List of initial users
        items:
            type: object
            description: User credentials
            additionalProperties: false
            properties:
                login:
                    type: string
                    description: User loginname
                pass:
                    type: string
                    description: User password
)");
}

} // namespace Auth
