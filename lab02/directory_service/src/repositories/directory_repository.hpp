#pragma once

#include <memory>
#include <optional>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/schema.hpp>
#include <vector>

#include "models/directory.hpp"
#include "models/file.hpp"
#include "utils/expected.hpp"

namespace Repositories {

enum class CreateDirectoryError {
    DirectoryAlreadyExists,
    ParentNotFound,
    ServerError
};

enum class UpdateDirectoryError {
    DirectoryNotFound,
    NameConflict,
    ServerError
};

enum class DeleteDirectoryError {
    DirectoryNotFound,
    DirectoryNotEmpty,
    ServerError
};

enum class MoveDirectoryError {
    DirectoryNotFound,
    ParentNotFound,
    WouldCreateCycle,
    NameConflict,
    ServerError
};

using CreateDirectoryResult = Common::Utils::expected<Models::Directory, CreateDirectoryError>;
using GetDirectoryResult = std::optional<Models::Directory>;
using UpdateDirectoryResult = Common::Utils::expected<Models::Directory, UpdateDirectoryError>;
using DeleteDirectoryResult = Common::Utils::expected<bool, DeleteDirectoryError>;
using MoveDirectoryResult = Common::Utils::expected<Models::Directory, MoveDirectoryError>;

struct DirectoryListParams {
    std::optional<std::string> parent_id;
    std::optional<int> limit;
    std::optional<int> offset;
};

struct DirectoryListResult {
    std::vector<Models::Directory> items;
    int total;
    int limit;
    int offset;
};

struct FileListParams {
    std::string directory_id;
    std::optional<int> limit;
    std::optional<int> offset;
};

struct FileListResult {
    std::vector<Models::File> items;
    int total;
    int limit;
    int offset;
};

class IDirectoryRepository {
  public:
    virtual ~IDirectoryRepository() = default;

    // Directory operations
    virtual CreateDirectoryResult CreateDirectory(const std::string &name,
                                                  const std::optional<std::string> &parent_id,
                                                  const std::string &owner_id) = 0;
    virtual GetDirectoryResult GetDirectory(const std::string &directory_id) = 0;
    virtual DirectoryListResult ListDirectories(const DirectoryListParams &params,
                                                const std::string &owner_id) = 0;
    virtual UpdateDirectoryResult UpdateDirectory(const std::string &directory_id,
                                                  const std::string &new_name,
                                                  const std::string &owner_id) = 0;
    virtual DeleteDirectoryResult DeleteDirectory(const std::string &directory_id,
                                                  bool recursive,
                                                  const std::string &owner_id) = 0;
    virtual MoveDirectoryResult MoveDirectory(const std::string &directory_id,
                                              const std::optional<std::string> &new_parent_id,
                                              const std::string &owner_id) = 0;

    // File operations
    virtual FileListResult ListFiles(const FileListParams &params,
                                     const std::string &owner_id) = 0;

    // Test support
    virtual void DeleteAll() = 0;
};

class DirectoryComponent : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "directory-repository";

    DirectoryComponent(const userver::components::ComponentConfig &config,
                       const userver::components::ComponentContext &context);

    std::shared_ptr<IDirectoryRepository> GetRepository();

    static userver::yaml_config::Schema GetStaticConfigSchema();

  private:
    std::shared_ptr<IDirectoryRepository> repository_;
};

} // namespace Repositories
