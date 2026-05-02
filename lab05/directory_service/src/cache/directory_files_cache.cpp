#include "directory_files_cache.hpp"

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

}  // namespace

DirectoryFilesCache::DirectoryFilesCache(userver::storages::redis::ClientPtr redis_client, std::chrono::seconds ttl)
    : redis_client_(std::move(redis_client))
    , redis_cc_(std::chrono::seconds{5}, std::chrono::seconds{5}, 1)
    , ttl_(ttl) {
}

std::optional<DirectoryFilesCache::Response> DirectoryFilesCache::Get(
    const std::string& owner_id,
    const std::string& directory_id,
    int limit,
    int offset) const {
    try {
        const auto result = redis_client_->Get(BuildKey(owner_id, directory_id, limit, offset), redis_cc_).Get();
        if (!result) {
            return std::nullopt;
        }

        return userver::formats::json::FromString(*result).As<Response>();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to read directory files cache: " << e.what();
        return std::nullopt;
    }
}

void DirectoryFilesCache::Set(
    const std::string& owner_id,
    const std::string& directory_id,
    int limit,
    int offset,
    const Response& response) const {
    try {
        const auto payload =
            userver::formats::json::ToString(userver::formats::json::ValueBuilder{response}.ExtractValue());

        redis_client_->Set(
            BuildKey(owner_id, directory_id, limit, offset),
            payload,
            std::chrono::duration_cast<std::chrono::milliseconds>(ttl_),
            redis_cc_)
            .Get();
    } catch (const std::exception& e) {
        LOG_WARNING() << "Failed to write directory files cache: " << e.what();
    }
}

std::string DirectoryFilesCache::BuildKey(
    const std::string& owner_id,
    const std::string& directory_id,
    int limit,
    int offset) {
    return "dir-files:" + owner_id + ":" + directory_id + ":" + std::to_string(limit) + ":" + std::to_string(offset);
}

DirectoryFilesCacheComponent::DirectoryFilesCacheComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : ComponentBase(config, context)
    , cache_(
          context.FindComponent<userver::components::Redis>(std::string{kRedisComponentName})
              .GetClient(std::string{kRedisDbName}),
          std::chrono::seconds{config["ttl-seconds"].As<int>()}) {
}

const DirectoryFilesCache& DirectoryFilesCacheComponent::GetCache() const { return cache_; }

}  // namespace Cache
