#include "file_repository.hpp"

#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/testsuite/tasks.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include "mongo_file_repository.hpp"

namespace Repositories {

FileComponent::FileComponent(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : ComponentBase(config, context) {
    auto &mongo_component = context.FindComponent<userver::components::Mongo>("mongo-files");
    auto pool = mongo_component.GetPool();
    repository_ = std::make_shared<MongoFileRepository>(std::move(pool));

    auto &testsuite_support = context.FindComponent<userver::components::TestsuiteSupport>();
    auto &tasks = testsuite_support.GetTestsuiteTasks();
    tasks.RegisterTask("delete-all-files", [repository = this->repository_]() {
        repository->DeleteAll();
    });

    LOG_INFO() << "FileComponent initialized with MongoDB";
}

std::shared_ptr<IFileRepository> FileComponent::GetRepository() {
    return repository_;
}

userver::yaml_config::Schema FileComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: File repository component
additionalProperties: false
"properties": {}
)");
}

} // namespace Repositories
