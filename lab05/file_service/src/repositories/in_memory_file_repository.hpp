#pragma once

#include <map>
#include <string>
#include <userver/concurrent/variable.hpp>

#include "repositories/file_repository.hpp"

namespace Repositories {

// Dummy implementation for testing. Cannot be fully implement without database
class InMemoryFileRepository : public IFileRepository {
  public:
    InMemoryFileRepository() = default;

    CreateFileResult CreateFile(const std::string &name,
                                const std::string &content,
                                const std::optional<std::string> &directory_id,
                                const std::string &owner_id) override;
    GetFileResult GetFile(const std::string &file_id) override;
    UpdateFileResult UpdateFile(const std::string &file_id,
                                const std::string &new_name,
                                const std::string &owner_id) override;
    DeleteFileResult DeleteFile(const std::string &file_id,
                                const std::string &owner_id) override;
    void DeleteAll() override;

  private:
    userver::concurrent::Variable<std::map<std::string, Models::File>> files_;
};

} // namespace Repositories
