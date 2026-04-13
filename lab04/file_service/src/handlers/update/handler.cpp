#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/file_repository.hpp"

namespace Handlers {

UpdateHandler::UpdateHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , file_repository_(context.FindComponent<Repositories::FileComponent>().GetRepository()) {
}

UpdateHandler::ResponseVariant
UpdateHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                  const Gen::openapi::FileUpdateRequest &body,
                                  userver::server::request::RequestContext &context) const {
    auto user_data = context.GetUserData<userver::formats::json::Value>();
    if (user_data.GetSize() == 0 || !user_data.HasMember("uuid") || !user_data.HasMember("login")) {
        return Error401("Unauthorized");
    }

    auto owner_id = user_data["uuid"].As<std::string>();

    const auto &file_id = request.GetPathArg("file_id");
    if (file_id.empty()) {
        return Error400("File ID is required");
    }

    if (body.name.empty()) {
        return Error400("File name is required");
    }

    auto result = file_repository_->UpdateFile(file_id, body.name, owner_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::UpdateFileError::FileNotFound:
            return Error404("File not found");
        case Repositories::UpdateFileError::NameConflict:
            return Error409("File with this name already exists");
        case Repositories::UpdateFileError::ServerError:
            return Error500("Internal server error");
        }
    }

    const auto &file = result.value();

    return Response{
        .id = userver::utils::BoostUuidFromString(file.id),
        .name = file.name,
        .size = static_cast<int>(file.size),
        .mime_type = file.mime_type,
        .directory_id = file.directory_id.has_value() ? std::optional<boost::uuids::uuid>(userver::utils::BoostUuidFromString(*file.directory_id)) : std::nullopt,
        .owner_id = userver::utils::BoostUuidFromString(file.owner_id),
        .created_at = file.created_at,
        .updated_at = file.updated_at,
        .status = Gen::openapi::File::Status::kAvailable};
}

} // namespace Handlers
