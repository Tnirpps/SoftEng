#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/redis/client.hpp>

#include "schemas/openapi.hpp"

namespace Cache {

class DirectoryCache final {
  public:
    using DirectoryResponse = Gen::openapi::Directory;
    using DirectoryListResponse = Gen::openapi::DirectoryListResponse;
    using DirectoryFilesResponse = Gen::openapi::FileListResponse;

    DirectoryCache(userver::storages::redis::ClientPtr redis_client,
                   std::chrono::seconds directory_ttl,
                   std::chrono::seconds directory_list_ttl,
                   std::chrono::seconds directory_files_ttl);

    std::optional<DirectoryResponse> GetDirectory(const std::string& owner_id,
                                                  const std::string& directory_id) const;
    void SetDirectory(const std::string& owner_id,
                      const std::string& directory_id,
                      const DirectoryResponse& response) const;
    void InvalidateDirectory(const std::string& owner_id, const std::string& directory_id) const;

    std::optional<DirectoryListResponse> GetDirectoryList(const std::string& owner_id,
                                                          const std::optional<std::string>& parent_id,
                                                          int limit,
                                                          int offset) const;
    void SetDirectoryList(const std::string& owner_id,
                          const std::optional<std::string>& parent_id,
                          int limit,
                          int offset,
                          const DirectoryListResponse& response) const;
    void InvalidateDirectoryList(const std::string& owner_id,
                                 const std::optional<std::string>& parent_id) const;

    std::optional<DirectoryFilesResponse> GetDirectoryFiles(const std::string& owner_id,
                                                            const std::string& directory_id,
                                                            int limit,
                                                            int offset) const;
    void SetDirectoryFiles(const std::string& owner_id,
                           const std::string& directory_id,
                           int limit,
                           int offset,
                           const DirectoryFilesResponse& response) const;
    void InvalidateDirectoryFiles(const std::string& owner_id, const std::string& directory_id) const;

  private:
    static std::string NormalizeParentId(const std::optional<std::string>& parent_id);
    static std::string BuildDirectoryKey(const std::string& owner_id, const std::string& directory_id);
    static std::string BuildDirectoryListVersionKey(const std::string& owner_id,
                                                    const std::optional<std::string>& parent_id);
    static std::string BuildDirectoryListKey(const std::string& owner_id,
                                             const std::optional<std::string>& parent_id,
                                             int limit,
                                             int offset,
                                             std::int64_t version);
    static std::string BuildDirectoryFilesVersionKey(const std::string& owner_id,
                                                     const std::string& directory_id);
    static std::string BuildDirectoryFilesKey(const std::string& owner_id,
                                              const std::string& directory_id,
                                              int limit,
                                              int offset,
                                              std::int64_t version);

    std::int64_t GetVersion(const std::string& key) const;
    void BumpVersion(const std::string& key) const;
    void DeleteKey(const std::string& key) const;

    template <typename Response>
    std::optional<Response> GetResponse(const std::string& key) const;

    template <typename Response>
    void SetResponse(const std::string& key, const Response& response, std::chrono::seconds ttl) const;

    userver::storages::redis::ClientPtr redis_client_;
    userver::storages::redis::CommandControl redis_cc_;
    std::chrono::seconds directory_ttl_;
    std::chrono::seconds directory_list_ttl_;
    std::chrono::seconds directory_files_ttl_;
};

class DirectoryCacheComponent final : public userver::components::ComponentBase {
  public:
    static constexpr std::string_view kName = "directory-response-cache";

    DirectoryCacheComponent(const userver::components::ComponentConfig& config,
                            const userver::components::ComponentContext& context);

    const DirectoryCache& GetCache() const;

  private:
    DirectoryCache cache_;
};

}  // namespace Cache
