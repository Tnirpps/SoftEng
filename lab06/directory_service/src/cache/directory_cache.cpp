#include "directory_cache.hpp"

#include <chrono>
#include <cstdint>
#include <utility>

#include <userver/components/component_config.hpp>
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/redis/component.hpp>

namespace Cache {

namespace {

constexpr std::string_view kRedisComponentName = "directory-cache";
constexpr std::string_view kRedisDbName = "directory-cache";
constexpr std::string_view kRootKey = "root";

template <typename Response>
std::optional<Response> ParseResponse(const std::optional<std::string>& payload) {
    if (!payload) {
        return std::nullopt;
    }

    return userver::formats::json::FromString(*payload).As<Response>();
}

template <typename Response>
std::string SerializeResponse(const Response& response) {
    return userver::formats::json::ToString(userver::formats::json::ValueBuilder{response}.ExtractValue());
}

}  // namespace

DirectoryCache::DirectoryCache(userver::storages::redis::ClientPtr redis_client,
                               std::chrono::seconds directory_ttl,
                               std::chrono::seconds directory_list_ttl,
                               std::chrono::seconds directory_files_ttl)
    : redis_client_(std::move(redis_client))
    , redis_cc_(std::chrono::seconds{5}, std::chrono::seconds{5}, 1)
    , directory_ttl_(directory_ttl)
    , directory_list_ttl_(directory_list_ttl)
    , directory_files_ttl_(directory_files_ttl) {
}

std::optional<DirectoryCache::DirectoryResponse> DirectoryCache::GetDirectory(const std::string& owner_id,
                                                                              const std::string& directory_id) const {
    return GetResponse<DirectoryResponse>(BuildDirectoryKey(owner_id, directory_id));
}

void DirectoryCache::SetDirectory(const std::string& owner_id,
                                  const std::string& directory_id,
                                  const DirectoryResponse& response) const {
    SetResponse(BuildDirectoryKey(owner_id, directory_id), response, directory_ttl_);
}

void DirectoryCache::InvalidateDirectory(const std::string& owner_id, const std::string& directory_id) const {
    DeleteKey(BuildDirectoryKey(owner_id, directory_id));
}

std::optional<DirectoryCache::DirectoryListResponse> DirectoryCache::GetDirectoryList(
    const std::string& owner_id,
    const std::optional<std::string>& parent_id,
    int limit,
    int offset) const {
    const auto version = GetVersion(BuildDirectoryListVersionKey(owner_id, parent_id));
    return GetResponse<DirectoryListResponse>(BuildDirectoryListKey(owner_id, parent_id, limit, offset, version));
}

void DirectoryCache::SetDirectoryList(const std::string& owner_id,
                                      const std::optional<std::string>& parent_id,
                                      int limit,
                                      int offset,
                                      const DirectoryListResponse& response) const {
    const auto version = GetVersion(BuildDirectoryListVersionKey(owner_id, parent_id));
    SetResponse(BuildDirectoryListKey(owner_id, parent_id, limit, offset, version), response, directory_list_ttl_);
}

void DirectoryCache::InvalidateDirectoryList(const std::string& owner_id,
                                             const std::optional<std::string>& parent_id) const {
    BumpVersion(BuildDirectoryListVersionKey(owner_id, parent_id));
}

std::optional<DirectoryCache::DirectoryFilesResponse> DirectoryCache::GetDirectoryFiles(
    const std::string& owner_id,
    const std::string& directory_id,
    int limit,
    int offset) const {
    const auto version = GetVersion(BuildDirectoryFilesVersionKey(owner_id, directory_id));
    return GetResponse<DirectoryFilesResponse>(BuildDirectoryFilesKey(owner_id, directory_id, limit, offset, version));
}

void DirectoryCache::SetDirectoryFiles(const std::string& owner_id,
                                       const std::string& directory_id,
                                       int limit,
                                       int offset,
                                       const DirectoryFilesResponse& response) const {
    const auto version = GetVersion(BuildDirectoryFilesVersionKey(owner_id, directory_id));
    SetResponse(BuildDirectoryFilesKey(owner_id, directory_id, limit, offset, version), response, directory_files_ttl_);
}

void DirectoryCache::InvalidateDirectoryFiles(const std::string& owner_id, const std::string& directory_id) const {
    BumpVersion(BuildDirectoryFilesVersionKey(owner_id, directory_id));
}

std::string DirectoryCache::NormalizeParentId(const std::optional<std::string>& parent_id) {
    return parent_id.value_or(std::string{kRootKey});
}

std::string DirectoryCache::BuildDirectoryKey(const std::string& owner_id, const std::string& directory_id) {
    return "dir:get:" + owner_id + ":" + directory_id;
}

std::string DirectoryCache::BuildDirectoryListVersionKey(const std::string& owner_id,
                                                         const std::optional<std::string>& parent_id) {
    return "dir:list:ver:" + owner_id + ":" + NormalizeParentId(parent_id);
}

std::string DirectoryCache::BuildDirectoryListKey(const std::string& owner_id,
                                                  const std::optional<std::string>& parent_id,
                                                  int limit,
                                                  int offset,
                                                  std::int64_t version) {
    return "dir:list:v" + std::to_string(version) + ":" + owner_id + ":" + NormalizeParentId(parent_id) + ":" +
           std::to_string(limit) + ":" + std::to_string(offset);
}

std::string DirectoryCache::BuildDirectoryFilesVersionKey(const std::string& owner_id,
                                                          const std::string& directory_id) {
    return "dir-files:ver:" + owner_id + ":" + directory_id;
}

std::string DirectoryCache::BuildDirectoryFilesKey(const std::string& owner_id,
                                                   const std::string& directory_id,
                                                   int limit,
                                                   int offset,
                                                   std::int64_t version) {
    return "dir-files:v" + std::to_string(version) + ":" + owner_id + ":" + directory_id + ":" +
           std::to_string(limit) + ":" + std::to_string(offset);
}

std::int64_t DirectoryCache::GetVersion(const std::string& key) const {
    try {
        const auto result = redis_client_->Get(key, redis_cc_).Get();
        if (!result) {
            return 0;
        }

        return std::stoll(*result);
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to read cache version for key '" << key << "': " << e.what();
        return 0;
    }
}

void DirectoryCache::BumpVersion(const std::string& key) const {
    try {
        redis_client_->Incr(key, redis_cc_).Get();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to bump cache version for key '" << key << "': " << e.what();
    }
}

void DirectoryCache::DeleteKey(const std::string& key) const {
    try {
        redis_client_->Del(key, redis_cc_).Get();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to delete cache key '" << key << "': " << e.what();
    }
}

template <typename Response>
std::optional<Response> DirectoryCache::GetResponse(const std::string& key) const {
    try {
        return ParseResponse<Response>(redis_client_->Get(key, redis_cc_).Get());
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to read cache key '" << key << "': " << e.what();
        return std::nullopt;
    }
}

template <typename Response>
void DirectoryCache::SetResponse(const std::string& key, const Response& response, std::chrono::seconds ttl) const {
    try {
        redis_client_->Set(
            key,
            SerializeResponse(response),
            std::chrono::duration_cast<std::chrono::milliseconds>(ttl),
            redis_cc_)
            .Get();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to write cache key '" << key << "': " << e.what();
    }
}

DirectoryCacheComponent::DirectoryCacheComponent(const userver::components::ComponentConfig& config,
                                                 const userver::components::ComponentContext& context)
    : ComponentBase(config, context)
    , cache_(
          context.FindComponent<userver::components::Redis>(std::string{kRedisComponentName})
              .GetClient(std::string{kRedisDbName}),
          std::chrono::seconds{config["directory-ttl-seconds"].As<int>()},
          std::chrono::seconds{config["directory-list-ttl-seconds"].As<int>()},
          std::chrono::seconds{config["directory-files-ttl-seconds"].As<int>()}) {
}

const DirectoryCache& DirectoryCacheComponent::GetCache() const { return cache_; }

template std::optional<DirectoryCache::DirectoryResponse> DirectoryCache::GetResponse<DirectoryResponse>(
    const std::string& key) const;
template std::optional<DirectoryCache::DirectoryListResponse> DirectoryCache::GetResponse<DirectoryListResponse>(
    const std::string& key) const;
template std::optional<DirectoryCache::DirectoryFilesResponse> DirectoryCache::GetResponse<DirectoryFilesResponse>(
    const std::string& key) const;

template void DirectoryCache::SetResponse<DirectoryCache::DirectoryResponse>(
    const std::string& key,
    const DirectoryCache::DirectoryResponse& response,
    std::chrono::seconds ttl) const;
template void DirectoryCache::SetResponse<DirectoryCache::DirectoryListResponse>(
    const std::string& key,
    const DirectoryCache::DirectoryListResponse& response,
    std::chrono::seconds ttl) const;
template void DirectoryCache::SetResponse<DirectoryCache::DirectoryFilesResponse>(
    const std::string& key,
    const DirectoryCache::DirectoryFilesResponse& response,
    std::chrono::seconds ttl) const;

}  // namespace Cache
