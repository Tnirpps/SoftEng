#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/directory_repository.hpp"

namespace Handlers {

GetHandler::GetHandler(const userver::components::ComponentConfig &config,
                       const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
    , directory_repository_(context.FindComponent<Repositories::DirectoryComponent>().GetRepository()) {
}

GetHandler::ResponseVariant
GetHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
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

    auto result = directory_repository_->GetDirectory(directory_id);

    if (!result.has_value()) {
        return Error404("Directory not found");
    }

    const auto &dir = result.value();

    if (dir.owner_id != owner_id) {
        return Error404("Directory not found");
    }

    return Response{
        .id = userver::utils::BoostUuidFromString(dir.id),
        .name = dir.name,
        .parent_id = dir.parent_id.has_value() ? std::optional<boost::uuids::uuid>(userver::utils::BoostUuidFromString(*dir.parent_id)) : std::nullopt,
        .owner_id = userver::utils::BoostUuidFromString(dir.owner_id),
        .created_at = dir.created_at,
        .updated_at = dir.updated_at,
        .is_root = dir.is_root};
}

} // namespace Handlers
