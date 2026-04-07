#pragma once

#include <string>

#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/logging/log.hpp>

#include "repositories/directory_repository.hpp"
#include <DirectoryService/sql_queries.hpp>

namespace Repositories {

class PostgresDirectoryRepository : public IDirectoryRepository {
  public:
    explicit PostgresDirectoryRepository(userver::storages::postgres::ClusterPtr cluster);

    // Directory operations
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

    // File operations
    FileListResult ListFiles(const FileListParams &params,
                             const std::string &owner_id) override;

    // Test support
    void DeleteAll() override;

  private:
    userver::storages::postgres::ClusterPtr cluster_;
};

} // namespace Repositories
