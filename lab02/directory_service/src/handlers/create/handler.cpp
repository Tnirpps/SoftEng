#include "handler.hpp"

#include <userver/formats/json/value.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>

#include "repositories/directory_repository.hpp"

namespace Handlers {

CreateHandler::CreateHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , directory_repository_(context.FindComponent<Repositories::DirectoryComponent>().GetRepository()) {
}

CreateHandler::ResponseVariant
CreateHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                  const Gen::openapi::DirectoryCreateRequest &body,
                                  userver::server::request::RequestContext &context) const {
    auto user_data = context.GetUserData<userver::formats::json::Value>();
    if (user_data.GetSize() == 0 || !user_data.HasMember("uuid") || !user_data.HasMember("login")) {
        return Error401("Unauthorized");
    }

    auto owner_id = user_data["uuid"].As<std::string>();

    std::optional<std::string> parent_id;
    if (body.parent_id.has_value()) {
        parent_id = userver::utils::ToString(body.parent_id.value());
    }

    auto result = directory_repository_->CreateDirectory(body.name, parent_id, owner_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::CreateDirectoryError::DirectoryAlreadyExists:
            return Error409("Directory with this name already exists");
        case Repositories::CreateDirectoryError::ParentNotFound:
            return Error400("Parent directory not found");
        case Repositories::CreateDirectoryError::ServerError:
            return Error500("Internal server error");
        }
    }

    const auto &dir = result.value();

    return Response{
        .id = userver::utils::BoostUuidFromString(dir.id),
        .name = dir.name,
        .parent_id = dir.parent_id.has_value() ? std::optional<boost::uuids::uuid>(userver::utils::BoostUuidFromString(*dir.parent_id)) : std::nullopt,
        .owner_id = userver::utils::BoostUuidFromString(owner_id),
        .created_at = dir.created_at,
        .updated_at = dir.updated_at,
        .is_root = dir.is_root};
}

} // namespace Handlers
