#include "auth_repository.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/testsuite/tasks.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "auth/repositories/in_memory_auth_repository.hpp"

namespace Auth {

AuthComponent::AuthComponent(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : ComponentBase(config, context)
    , repository_(std::make_shared<Repositories::InMemoryAuthRepository>()) {
    auto &testsuite_support = context.FindComponent<userver::components::TestsuiteSupport>();
    auto &tasks = testsuite_support.GetTestsuiteTasks();
    tasks.RegisterTask("delete-all-users", [repository = this->repository_]() {
        repository->DeleteAllUsers();
    });

    LOG_INFO() << "AuthComponent initialized";
}

std::shared_ptr<IAuthRepository> AuthComponent::GetRepository() {
    return repository_;
}

userver::yaml_config::Schema AuthComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: Auth repository component
additionalProperties: false
"properties": {}
)");
}

} // namespace Auth
