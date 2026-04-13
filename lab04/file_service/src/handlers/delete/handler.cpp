#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/file_repository.hpp"

namespace Handlers {

DeleteHandler::DeleteHandler(const userver::components::ComponentConfig &config,
                             const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
    , file_repository_(context.FindComponent<Repositories::FileComponent>().GetRepository()) {
}

DeleteHandler::ResponseVariant
DeleteHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
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

    auto result = file_repository_->DeleteFile(file_id, owner_id);

    if (!result.has_value()) {
        switch (result.error()) {
        case Repositories::DeleteFileError::FileNotFound:
            return Error404("File not found");
        case Repositories::DeleteFileError::ServerError:
            return Error500("Internal server error");
        }
    }

    return Response{
        .message = "File successfully deleted"};
}

} // namespace Handlers
