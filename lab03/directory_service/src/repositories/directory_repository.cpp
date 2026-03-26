#include "directory_repository.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/testsuite/tasks.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "in_memory_directory_repository.hpp"

namespace Repositories {

DirectoryComponent::DirectoryComponent(const userver::components::ComponentConfig &config,
                                       const userver::components::ComponentContext &context)
    : ComponentBase(config, context)
    , repository_(std::make_shared<InMemoryDirectoryRepository>()) {
    auto &testsuite_support = context.FindComponent<userver::components::TestsuiteSupport>();
    auto &tasks = testsuite_support.GetTestsuiteTasks();
    tasks.RegisterTask("delete-all-directories", [repository = this->repository_]() {
        repository->DeleteAll();
    });

    LOG_INFO() << "DirectoryComponent initialized";
}

std::shared_ptr<IDirectoryRepository> DirectoryComponent::GetRepository() {
    return repository_;
}

userver::yaml_config::Schema DirectoryComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: Directory repository component
additionalProperties: false
"properties": {}
)");
}

} // namespace Repositories
