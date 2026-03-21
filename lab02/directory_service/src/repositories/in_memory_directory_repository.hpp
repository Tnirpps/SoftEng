#pragma once

#include <map>
#include <string>
#include <userver/concurrent/variable.hpp>

#include "repositories/directory_repository.hpp"

namespace Repositories {

class InMemoryDirectoryRepository : public IDirectoryRepository {
  public:
    InMemoryDirectoryRepository() = default;

    CreateDirectoryResult CreateDirectory(const std::string &name,
                                          const std::optional<std::string> &parent_id,
                                          const std::string &owner_id) override;
    GetDirectoryResult GetDirectory(const std::string &directory_id) override;
    DirectoryListResult ListDirectories(const DirectoryListParams &params,
                                        const std::string &owner_id) override;
    UpdateDirectoryResult UpdateDirectory(const std::string &directory_id,
                                          const std::string &new_name,
                                          const std::string &owner_id) override;
    DeleteDirectoryResult DeleteDirectory(const std::string &directory_id,
                                          bool recursive,
                                          const std::string &owner_id) override;
    MoveDirectoryResult MoveDirectory(const std::string &directory_id,
                                      const std::optional<std::string> &new_parent_id,
                                      const std::string &owner_id) override;
    FileListResult ListFiles(const FileListParams &params,
                             const std::string &owner_id) override;
    void DeleteAll() override;

  private:
    userver::concurrent::Variable<std::map<std::string, Models::Directory>> directories_;
    userver::concurrent::Variable<std::map<std::string, Models::File>> files_;
};

} // namespace Repositories
