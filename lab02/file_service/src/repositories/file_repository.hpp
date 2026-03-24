#pragma once

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/schema.hpp>

#include "models/file.hpp"

namespace Repositories {

enum class CreateFileError {
    FileAlreadyExists,
    ParentNotFound,
    ServerError
};

enum class UpdateFileError {
    FileNotFound,
    NameConflict,
    ServerError
};

enum class DeleteFileError {
    FileNotFound,
    ServerError
};

enum class GetFileError {
    FileNotFound,
    ServerError
};

using CreateFileResult = std::expected<Models::File, CreateFileError>;
using GetFileResult = std::expected<Models::File, GetFileError>;
using UpdateFileResult = std::expected<Models::File, UpdateFileError>;
using DeleteFileResult = std::expected<bool, DeleteFileError>;

struct FileListParams {
    std::optional<std::string> directory_id;
    std::optional<int> limit;
    std::optional<int> offset;
};

class IFileRepository {
  public:
    virtual ~IFileRepository() = default;

    // File operations
    virtual CreateFileResult CreateFile(const std::string &name,
                                        const std::string &content,
                                        const std::optional<std::string> &directory_id,
                                        const std::string &owner_id) = 0;
    virtual GetFileResult GetFile(const std::string &file_id) = 0;
    virtual UpdateFileResult UpdateFile(const std::string &file_id,
                                        const std::string &new_name,
                                        const std::string &owner_id) = 0;
    virtual DeleteFileResult DeleteFile(const std::string &file_id,
                                        const std::string &owner_id) = 0;

    // Test support
    virtual void DeleteAll() = 0;
};

class FileComponent : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "file-repository";

    FileComponent(const userver::components::ComponentConfig &config,
                  const userver::components::ComponentContext &context);

    std::shared_ptr<IFileRepository> GetRepository();

    static userver::yaml_config::Schema GetStaticConfigSchema();

  private:
    std::shared_ptr<IFileRepository> repository_;
};

} // namespace Repositories
