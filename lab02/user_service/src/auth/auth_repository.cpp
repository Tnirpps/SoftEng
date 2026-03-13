#include "auth_repository.hpp"

#include <memory>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/yaml_config/schema.hpp>

namespace auth {

// ==================== InMemoryAuthRepository ====================

InMemoryAuthRepository::InMemoryAuthRepository(const std::map<std::string, std::string> &initial_users)
    : users_(initial_users) {
}

bool InMemoryAuthRepository::CheckUser(const std::string &nick, const std::string &password) {
    // Use Lock() for read-only access - efficient O(log N) lookup
    auto data_ptr = users_.Lock();
    auto it = data_ptr->find(nick);
    return it != data_ptr->end() && it->second == password;
}

bool InMemoryAuthRepository::AddUser(const std::string &nick, const std::string &password) {
    // Use Lock() for thread-safe write access
    auto data_ptr = users_.Lock();
    auto [it, inserted] = data_ptr->emplace(nick, password);
    return inserted; // Returns true if insertion succeeded (user didn't exist)
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
            const std::string nick = user_entry["nick"].As<std::string>();
            const std::string pass = user_entry["pass"].As<std::string>();
            repository_->AddUser(nick, pass);
        }
    }

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
                nick:
                    type: string
                    description: User nickname
                pass:
                    type: string
                    description: User password
)");
}

} // namespace auth
