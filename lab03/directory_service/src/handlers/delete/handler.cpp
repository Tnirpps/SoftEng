#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/directory_repository.hpp"
#include "utils/request_args.hpp"

namespace Handlers {

DeleteHandler::DeleteHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
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

    return Response{
        .message = "Directory successfully deleted"};
}

} // namespace Handlers
