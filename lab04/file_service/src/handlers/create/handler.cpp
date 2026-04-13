#include "handler.hpp"

#include <optional>
#include <userver/formats/json/value.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>

#include "repositories/file_repository.hpp"

namespace Handlers {

CreateHandler::CreateHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedJsonHandler(config, context)
    , file_repository_(context.FindComponent<Repositories::FileComponent>().GetRepository()) {
}

CreateHandler::ResponseVariant
CreateHandler::HandleTypedRequest(const userver::server::http::HttpRequest & /*request*/,
                                  const Gen::openapi::FileCreateRequest &body,
                                  userver::server::request::RequestContext &context) const {
    auto user_data = context.GetUserData<userver::formats::json::Value>();
    if (user_data.GetSize() == 0 || !user_data.HasMember("uuid") || !user_data.HasMember("login")) {
        return Error401("Unauthorized");
    }

    auto owner_id = user_data["uuid"].As<std::string>();

    std::optional<std::string> directory_id;
    if (body.parent_id.has_value()) {
        directory_id = userver::utils::ToString(body.parent_id.value());
    }

    auto result = file_repository_->CreateFile(body.name, body.content, directory_id, owner_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::CreateFileError::FileAlreadyExists:
            return Error409("File with this name already exists");
        case Repositories::CreateFileError::ParentNotFound:
            return Error400("Parent directory not found");
        case Repositories::CreateFileError::ServerError:
            return Error500("Internal server error");
        }
    }

    const auto &file = result.value();

    return Response{
        .id = userver::utils::BoostUuidFromString(file.id),
        .name = file.name,
        .size = static_cast<int>(file.size),
        .mime_type = file.mime_type,
        .directory_id = file.directory_id.has_value() ? std::optional(userver::utils::BoostUuidFromString(*file.directory_id)) : std::nullopt,
        .owner_id = userver::utils::BoostUuidFromString(file.owner_id),
        .created_at = file.created_at,
        .updated_at = file.updated_at,
        .status = [&file]() {
            switch (file.status) {
            case Models::FileStatus::Pending:
                return Gen::openapi::File::Status::kPending;
            case Models::FileStatus::Scanning:
                return Gen::openapi::File::Status::kScanning;
            case Models::FileStatus::Available:
                return Gen::openapi::File::Status::kAvailable;
            case Models::FileStatus::Infected:
                return Gen::openapi::File::Status::kInfected;
            }
        }()};
}

} // namespace Handlers
