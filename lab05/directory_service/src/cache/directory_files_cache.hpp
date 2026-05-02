#pragma once

#include <chrono>
#include <optional>
#include <string>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/redis/client.hpp>

#include "schemas/openapi.hpp"

namespace Cache {

class DirectoryFilesCache final {
  public:
    using Response = Gen::openapi::FileListResponse;

    DirectoryFilesCache(userver::storages::redis::ClientPtr redis_client, std::chrono::seconds ttl);

    std::optional<Response> Get(const std::string& owner_id,
                                const std::string& directory_id,
                                int limit,
                                int offset) const;
    void Set(const std::string& owner_id,
             const std::string& directory_id,
             int limit,
             int offset,
             const Response& response) const;

  private:
    static std::string BuildKey(const std::string& owner_id,
                                const std::string& directory_id,
                                int limit,
                                int offset);

    userver::storages::redis::ClientPtr redis_client_;
    userver::storages::redis::CommandControl redis_cc_;
    std::chrono::seconds ttl_;
};

class DirectoryFilesCacheComponent final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "directory-files-cache";

    DirectoryFilesCacheComponent(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context);

    const DirectoryFilesCache& GetCache() const;

  private:
    DirectoryFilesCache cache_;
};

}  // namespace Cache
