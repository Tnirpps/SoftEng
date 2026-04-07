#include "handler.hpp"

#include <userver/utils/boost_uuid4.hpp>

#include "repositories/directory_repository.hpp"
#include "utils/request_args.hpp"

namespace Handlers {

ListHandler::ListHandler(const userver::components::ComponentConfig &config,
                         const userver::components::ComponentContext &context)
    : TypedHandler(config, context)
    , directory_repository_(context.FindComponent<Repositories::DirectoryComponent>().GetRepository()) {
}

ListHandler::ResponseVariant
ListHandler::HandleTypedRequest(const userver::server::http::HttpRequest &request,
                                userver::server::request::RequestContext &context) const {
    auto user_data = context.GetUserData<userver::formats::json::Value>();
    if (user_data.GetSize() == 0 || !user_data.HasMember("uuid") || !user_data.HasMember("login")) {
        return Error401("Unauthorized");
    }

    auto owner_id = user_data["uuid"].As<std::string>();

    std::optional<std::string> parent_id;
    const auto &parent_id_str = request.GetArg("parent_id");
    if (!parent_id_str.empty()) {
        parent_id = parent_id_str;
    }

    auto limit_parsed = Utils::ParseIntArg(request.GetArg("limit"));
    int limit = limit_parsed.value_or(20);

    auto offset_parsed = Utils::ParseIntArg(request.GetArg("offset"));
    int offset = offset_parsed.value_or(0);

    Repositories::DirectoryListParams params{
        .parent_id = parent_id,
        .limit = limit,
        .offset = offset};

    auto result = directory_repository_->ListDirectories(params, owner_id);

    std::vector<Gen::openapi::Directory> items;
    items.reserve(result.items.size());
    for (const auto &dir : result.items) {
        items.push_back(Gen::openapi::Directory{
            .id = dir.uuid,
            .name = dir.name,
            .parent_id = dir.parent_uuid,
            .owner_id = dir.owner_uuid,
            .created_at = userver::utils::datetime::TimePointTz(dir.created_at),
            .updated_at = userver::utils::datetime::TimePointTz(dir.updated_at),
            .is_root = dir.is_root});
    }

    return Response{
        .items = std::move(items),
        .total = result.total,
        .limit = result.limit,
        .offset = result.offset};
}

} // namespace Handlers
