#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/directory_repository.hpp"

namespace Handlers {

UpdateHandler::UpdateHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , directory_repository_(context.FindComponent<Repositories::DirectoryComponent>().GetRepository()) {
}

UpdateHandler::ResponseVariant
UpdateHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                  const Gen::openapi::DirectoryUpdateRequest &body,
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

    auto result = directory_repository_->UpdateDirectory(directory_id, body.name, owner_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::UpdateDirectoryError::DirectoryNotFound:
            return Error404("Directory not found");
        case Repositories::UpdateDirectoryError::NameConflict:
            return Error409("Directory with this name already exists");
        case Repositories::UpdateDirectoryError::ServerError:
            return Error500("Internal server error");
        }
    }

    const auto &dir = result.value();

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
