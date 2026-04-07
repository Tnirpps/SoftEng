#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/directory_repository.hpp"
#include "utils/request_args.hpp"

namespace Handlers {

FilesListHandler::FilesListHandler(const userver::components::ComponentConfig &config,
                                   const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
    , directory_repository_(context.FindComponent<Repositories::DirectoryComponent>().GetRepository()) {
}

FilesListHandler::ResponseVariant
FilesListHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
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

    auto limit_parsed = Utils::ParseIntArg(request.GetArg("limit"));
    int limit = limit_parsed.value_or(20);

    auto offset_parsed = Utils::ParseIntArg(request.GetArg("offset"));
    int offset = offset_parsed.value_or(0);

    // First check if directory exists and belongs to user
    auto dir_result = directory_repository_->GetDirectory(directory_id);
    if (!dir_result.has_value() || dir_result->owner_uuid != userver::utils::BoostUuidFromString(owner_id)) {
        return Error404("Directory not found");
    }

    Repositories::FileListParams params{
        .directory_id = directory_id,
        .limit = limit,
        .offset = offset};

    auto result = directory_repository_->ListFiles(params, owner_id);

    std::vector<Gen::openapi::FileMetadata> items;
    items.reserve(result.items.size());
    for (const auto &file : result.items) {
        items.push_back(Gen::openapi::FileMetadata{
            .id = file.uuid,
            .name = file.name,
            .size = static_cast<int>(file.size),
            .mime_type = file.mime_type,
            .directory_id = file.directory_uuid,
            .owner_id = file.owner_uuid,
            .created_at = userver::utils::datetime::TimePointTz(file.created_at),
            .updated_at = userver::utils::datetime::TimePointTz(file.updated_at),
            .status = static_cast<Gen::openapi::FileMetadata::Status>(Models::FileStatus::Pending)});// file.status)});
    }

    return Response{
        .items = std::move(items),
        .total = result.total,
        .limit = result.limit,
        .offset = result.offset};
}

} // namespace Handlers
