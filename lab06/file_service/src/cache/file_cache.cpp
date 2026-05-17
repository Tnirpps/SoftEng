#include "file_cache.hpp"

#include <chrono>
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

FileCache::FileCache(userver::storages::redis::ClientPtr redis_client, std::chrono::seconds file_ttl)
    : redis_client_(std::move(redis_client))
    , redis_cc_(std::chrono::seconds{5}, std::chrono::seconds{5}, 1)
    , file_ttl_(file_ttl) {
}

std::optional<FileCache::Response> FileCache::GetFile(const std::string& owner_id, const std::string& file_id) const {
    try {
        return ParseResponse<Response>(redis_client_->Get(BuildFileKey(owner_id, file_id), redis_cc_).Get());
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to read file cache: " << e.what();
        return std::nullopt;
    }
}

void FileCache::SetFile(const std::string& owner_id, const std::string& file_id, const Response& response) const {
    try {
        redis_client_->Set(
            BuildFileKey(owner_id, file_id),
            SerializeResponse(response),
            std::chrono::duration_cast<std::chrono::milliseconds>(file_ttl_),
            redis_cc_)
            .Get();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to write file cache: " << e.what();
    }
}

void FileCache::InvalidateFile(const std::string& owner_id, const std::string& file_id) const {
    try {
        redis_client_->Del(BuildFileKey(owner_id, file_id), redis_cc_).Get();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to delete file cache: " << e.what();
    }
}

void FileCache::InvalidateDirectoryFiles(const std::string& owner_id, const std::string& directory_id) const {
    try {
        redis_client_->Incr(BuildDirectoryFilesVersionKey(owner_id, directory_id), redis_cc_).Get();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to invalidate directory files cache: " << e.what();
    }
}

std::string FileCache::BuildFileKey(const std::string& owner_id, const std::string& file_id) {
    return "file:get:" + owner_id + ":" + file_id;
}

std::string FileCache::BuildDirectoryFilesVersionKey(const std::string& owner_id, const std::string& directory_id) {
    return "dir-files:ver:" + owner_id + ":" + directory_id;
}

FileCacheComponent::FileCacheComponent(const userver::components::ComponentConfig& config,
                                       const userver::components::ComponentContext& context)
    : ComponentBase(config, context)
    , cache_(
          context.FindComponent<userver::components::Redis>(std::string{kRedisComponentName})
              .GetClient(std::string{kRedisDbName}),
          std::chrono::seconds{config["file-ttl-seconds"].As<int>()}) {
}

const FileCache& FileCacheComponent::GetCache() const { return cache_; }

}  // namespace Cache
