#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/file_repository.hpp"

namespace Handlers {

GetHandler::GetHandler(const userver::components::ComponentConfig &config,
                       const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
    , cache_(context.FindComponent<Cache::FileCacheComponent>().GetCache())
    , file_repository_(context.FindComponent<Repositories::FileComponent>().GetRepository()) {
}

GetHandler::ResponseVariant
GetHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
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

    if (const auto cached_response = cache_.GetFile(owner_id, file_id)) {
        return *cached_response;
    }

    auto result = file_repository_->GetFile(file_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::GetFileError::FileNotFound:
            return Error404("File not found");
        case Repositories::GetFileError::ServerError:
            return Error500("Internal server error");
        }
    }

    const auto &file = result.value();

    if (file.owner_id != owner_id) {
        return Error404("File not found");
    }

    Response response{
        .id = userver::utils::BoostUuidFromString(file.id),
        .name = file.name,
        .size = static_cast<int>(file.size),
        .mime_type = file.mime_type,
        .directory_id = file.directory_id.has_value() ? std::optional<boost::uuids::uuid>(userver::utils::BoostUuidFromString(*file.directory_id)) : std::nullopt,
        .owner_id = userver::utils::BoostUuidFromString(file.owner_id),
        .created_at = file.created_at,
        .updated_at = file.updated_at,
        .status = Gen::openapi::File::Status::kAvailable};

    cache_.SetFile(owner_id, file_id, response);
    return response;
}

} // namespace Handlers
