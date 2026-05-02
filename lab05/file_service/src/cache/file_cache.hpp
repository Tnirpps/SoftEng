#pragma once

#include <chrono>
#include <optional>
#include <string>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/redis/client.hpp>

#include "schemas/openapi.hpp"

namespace Cache {

class FileCache final {
  public:
    using Response = Gen::openapi::File;

    FileCache(userver::storages::redis::ClientPtr redis_client, std::chrono::seconds file_ttl);

    std::optional<Response> GetFile(const std::string& owner_id, const std::string& file_id) const;
    void SetFile(const std::string& owner_id, const std::string& file_id, const Response& response) const;
    void InvalidateFile(const std::string& owner_id, const std::string& file_id) const;

    void InvalidateDirectoryFiles(const std::string& owner_id, const std::string& directory_id) const;

  private:
    static std::string BuildFileKey(const std::string& owner_id, const std::string& file_id);
    static std::string BuildDirectoryFilesVersionKey(const std::string& owner_id, const std::string& directory_id);

    userver::storages::redis::ClientPtr redis_client_;
    userver::storages::redis::CommandControl redis_cc_;
    std::chrono::seconds file_ttl_;
};

class FileCacheComponent final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "file-response-cache";

    FileCacheComponent(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& context);

    const FileCache& GetCache() const;

  private:
    FileCache cache_;
};

}  // namespace Cache
