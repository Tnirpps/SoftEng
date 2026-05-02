#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/directory_repository.hpp"

namespace Handlers {

MoveHandler::MoveHandler(const userver::components::ComponentConfig &config,
                         const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , cache_(context.FindComponent<Cache::DirectoryCacheComponent>().GetCache())
    , directory_repository_(context.FindComponent<Repositories::DirectoryComponent>().GetRepository()) {
}

MoveHandler::ResponseVariant
MoveHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                const Gen::openapi::DirectoryMoveRequest &body,
                                userver::server::request::RequestContext &context) const {
    auto user_data = context.GetUserData<userver::formats::json::Value>();
    if (user_data.GetSize() == 0 || !user_data.HasMember("uuid") || !user_data.HasMember("login")) {
        return Error401("Unauthorized");
    }

    auto owner_id = user_data["uuid"].As<std::string>();

    const auto &directory_id = request.GetPathArg("directory_id");
    if (directory_id.empty()) {
        return Error400("Directory ID is required");
    }

    std::optional<std::string> new_parent_id;
    if (body.new_parent_id.has_value()) {
        new_parent_id = userver::utils::ToString(body.new_parent_id.value());
    }

    std::optional<std::string> old_parent_id;
    if (const auto existing_directory = directory_repository_->GetDirectory(directory_id);
        existing_directory.has_value() &&
        existing_directory->owner_uuid == userver::utils::BoostUuidFromString(owner_id)) {
        if (existing_directory->parent_uuid) {
            old_parent_id = userver::utils::ToString(*existing_directory->parent_uuid);
        }
    }

    auto result = directory_repository_->MoveDirectory(directory_id, new_parent_id, owner_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::MoveDirectoryError::DirectoryNotFound:
            return Error404("Directory not found");
        case Repositories::MoveDirectoryError::ParentNotFound:
            return Error404("New parent directory not found");
        case Repositories::MoveDirectoryError::WouldCreateCycle:
            return Error400("Cannot move directory into itself or its descendant");
        case Repositories::MoveDirectoryError::NameConflict:
            return Error409("Directory with this name already exists in target location");
        case Repositories::MoveDirectoryError::ServerError:
            return Error500("Internal server error");
        }
    }

    const auto &dir = result.value();

    cache_.InvalidateDirectory(owner_id, directory_id);
    cache_.InvalidateDirectoryList(owner_id, old_parent_id);
    cache_.InvalidateDirectoryList(
        owner_id, dir.parent_uuid ? std::optional(userver::utils::ToString(*dir.parent_uuid)) : std::nullopt);

    return Response{
        .id = dir.uuid,
        .name = dir.name,
        .parent_id = dir.parent_uuid,
        .owner_id = dir.owner_uuid,
        .created_at = userver::utils::datetime::TimePointTz(dir.created_at),
        .updated_at = userver::utils::datetime::TimePointTz(dir.updated_at),
        .is_root = dir.is_root};
}

} // namespace Handlers
