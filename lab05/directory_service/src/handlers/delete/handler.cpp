#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/directory_repository.hpp"
#include "utils/request_args.hpp"

namespace Handlers {

DeleteHandler::DeleteHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
    , cache_(context.FindComponent<Cache::DirectoryCacheComponent>().GetCache())
    , directory_repository_(context.FindComponent<Repositories::DirectoryComponent>().GetRepository()) {
}

DeleteHandler::ResponseVariant
DeleteHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
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

    bool recursive = Utils::GetBoolArg(request.GetArg("recursive"), false);
    std::optional<std::string> parent_id_for_invalidation;

    if (const auto existing_directory = directory_repository_->GetDirectory(directory_id);
        existing_directory.has_value() &&
        existing_directory->owner_uuid == userver::utils::BoostUuidFromString(owner_id)) {
        if (existing_directory->parent_uuid) {
            parent_id_for_invalidation = userver::utils::ToString(*existing_directory->parent_uuid);
        }
    }

    auto result = directory_repository_->DeleteDirectory(directory_id, recursive, owner_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::DeleteDirectoryError::DirectoryNotFound:
            return Error404("Directory not found");
        case Repositories::DeleteDirectoryError::DirectoryNotEmpty:
            return Error400("Directory is not empty and recursive=false");
        case Repositories::DeleteDirectoryError::ServerError:
            return Error500("Internal server error");
        }
    }

    cache_.InvalidateDirectory(owner_id, directory_id);
    cache_.InvalidateDirectoryList(owner_id, parent_id_for_invalidation);
    cache_.InvalidateDirectoryFiles(owner_id, directory_id);

    return Response{
        .message = "Directory successfully deleted"};
}

} // namespace Handlers
