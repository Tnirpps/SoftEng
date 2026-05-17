#pragma once

#include <string>
#include <userver/storages/mongo/component.hpp>

#include "file_repository.hpp"
#include "repositories/file_repository.hpp"

namespace Repositories {

class MongoFileRepository : public IFileRepository {
  public:
    explicit MongoFileRepository(userver::storages::mongo::PoolPtr pool);

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
    userver::storages::mongo::PoolPtr pool_;
    std::string collection_name_ = "files";
};

} // namespace Repositories
