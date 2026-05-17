#include "mongo_file_repository.hpp"

#include <userver/formats/bson/inline.hpp>
#include <userver/logging/log.hpp>
#include <userver/storages/mongo/exception.hpp>
#include <userver/utils/boost_uuid4.hpp>
#include <userver/utils/datetime.hpp>
#include <userver/utils/datetime/timepoint_tz.hpp>

#include "utils/expected.hpp"
#include "utils/uuid_generator.hpp"

namespace Repositories {

namespace {

std::string GetMimeType(const std::string &name) {
    if (name.ends_with(".txt"))
        return "text/plain";
    if (name.ends_with(".json"))
        return "application/json";
    if (name.ends_with(".xml"))
        return "application/xml";
    if (name.ends_with(".html") || name.ends_with(".htm"))
        return "text/html";
    if (name.ends_with(".css"))
        return "text/css";
    if (name.ends_with(".js"))
        return "application/javascript";
    if (name.ends_with(".png"))
        return "image/png";
    if (name.ends_with(".jpg") || name.ends_with(".jpeg"))
        return "image/jpeg";
    if (name.ends_with(".gif"))
        return "image/gif";
    if (name.ends_with(".pdf"))
        return "application/pdf";
    if (name.ends_with(".md"))
        return "text/markdown";
    return "application/octet-stream";
}

std::string FileStatusToString(Models::FileStatus status) {
    switch (status) {
    case Models::FileStatus::Pending:
        return "pending";
    case Models::FileStatus::Scanning:
        return "scanning";
    case Models::FileStatus::Available:
        return "available";
    case Models::FileStatus::Infected:
        return "infected";
    }
    return "unknown";
}

Models::FileStatus StringToFileStatus(const std::string &str) {
    if (str == "pending")
        return Models::FileStatus::Pending;
    if (str == "scanning")
        return Models::FileStatus::Scanning;
    if (str == "available")
        return Models::FileStatus::Available;
    if (str == "infected")
        return Models::FileStatus::Infected;
    return Models::FileStatus::Available;
}

} // namespace

MongoFileRepository::MongoFileRepository(userver::storages::mongo::PoolPtr pool)
    : pool_(std::move(pool)) {
}

CreateFileResult MongoFileRepository::CreateFile(
    const std::string &name,
    const std::string &content,
    const std::optional<std::string> &directory_id,
    const std::string &owner_id) {

    try {
        auto collection = pool_->GetCollection(collection_name_);

        using userver::formats::bson::MakeDoc;

        // Check if file with same name and directory already exists
        auto check_query = directory_id.has_value()
                               ? MakeDoc("owner_id", owner_id, "name", name, "directory_id", *directory_id)
                               : MakeDoc("owner_id", owner_id, "name", name, "directory_id", MakeDoc("$exists", false));

        auto existing = collection.FindOne(check_query);
        if (existing) {
            return Common::Utils::unexpected(CreateFileError::FileAlreadyExists);
        }

        std::string new_id = Utils::GenerateUuid();
        auto now = userver::utils::datetime::Now();

        // Create file document
        auto file_doc = directory_id.has_value()
                            ? MakeDoc(
                                  "id", new_id,
                                  "name", name,
                                  "size", static_cast<std::int64_t>(content.size()),
                                  "mime_type", GetMimeType(name),
                                  "directory_id", *directory_id,
                                  "owner_id", owner_id,
                                  "created_at", now,
                                  "updated_at", now,
                                  "status", FileStatusToString(Models::FileStatus::Available),
                                  "content", content)
                            : MakeDoc(
                                  "id", new_id,
                                  "name", name,
                                  "size", static_cast<std::int64_t>(content.size()),
                                  "mime_type", GetMimeType(name),
                                  "owner_id", owner_id,
                                  "created_at", now,
                                  "updated_at", now,
                                  "status", FileStatusToString(Models::FileStatus::Available),
                                  "content", content);

        collection.InsertOne(file_doc);

        Models::File new_file{
            .id = new_id,
            .name = name,
            .size = static_cast<std::int64_t>(content.size()),
            .mime_type = GetMimeType(name),
            .directory_id = directory_id,
            .owner_id = owner_id,
            .created_at = userver::utils::datetime::TimePointTz{now},
            .updated_at = userver::utils::datetime::TimePointTz{now},
            .status = Models::FileStatus::Available};

        return new_file;
    } catch (const userver::storages::mongo::MongoException &e) {
        LOG_ERROR() << "MongoDB error in CreateFile: " << e.what();
        return Common::Utils::unexpected(CreateFileError::ServerError);
    } catch (const std::exception &e) {
        LOG_ERROR() << "Error in CreateFile: " << e.what();
        return Common::Utils::unexpected(CreateFileError::ServerError);
    }
}

GetFileResult MongoFileRepository::GetFile(const std::string &file_id) {
    try {
        auto collection = pool_->GetCollection(collection_name_);

        using userver::formats::bson::MakeDoc;
        auto doc_opt = collection.FindOne(MakeDoc("id", file_id));

        if (!doc_opt) {
            return Common::Utils::unexpected(GetFileError::FileNotFound);
        }

        auto doc = std::move(*doc_opt);

        std::optional<std::string> directory_id;
        if (doc.HasMember("directory_id") && !doc["directory_id"].IsNull()) {
            directory_id = doc["directory_id"].As<std::string>();
        }

        auto created_at_tp = doc["created_at"].As<std::chrono::system_clock::time_point>();
        auto updated_at_tp = doc["updated_at"].As<std::chrono::system_clock::time_point>();

        Models::File file{
            .id = doc["id"].As<std::string>(),
            .name = doc["name"].As<std::string>(),
            .size = doc["size"].As<std::int64_t>(),
            .mime_type = doc["mime_type"].As<std::string>(),
            .directory_id = directory_id,
            .owner_id = doc["owner_id"].As<std::string>(),
            .created_at = userver::utils::datetime::TimePointTz{created_at_tp},
            .updated_at = userver::utils::datetime::TimePointTz{updated_at_tp},
            .status = StringToFileStatus(doc["status"].As<std::string>())};

        return file;
    } catch (const userver::storages::mongo::MongoException &e) {
        LOG_ERROR() << "MongoDB error in GetFile: " << e.what();
        return Common::Utils::unexpected(GetFileError::ServerError);
    } catch (const std::exception &e) {
        LOG_ERROR() << "Error in GetFile: " << e.what();
        return Common::Utils::unexpected(GetFileError::ServerError);
    }
}

UpdateFileResult MongoFileRepository::UpdateFile(
    const std::string &file_id,
    const std::string &new_name,
    const std::string &owner_id) {

    try {
        auto collection = pool_->GetCollection(collection_name_);

        using userver::formats::bson::MakeDoc;

        // Get current file
        auto current_doc_opt = collection.FindOne(MakeDoc("id", file_id, "owner_id", owner_id));
        if (!current_doc_opt) {
            return Common::Utils::unexpected(UpdateFileError::FileNotFound);
        }

        auto current_doc = std::move(*current_doc_opt);

        std::optional<std::string> directory_id;
        if (current_doc.HasMember("directory_id") && !current_doc["directory_id"].IsNull()) {
            directory_id = current_doc["directory_id"].As<std::string>();
        }

        // Check for name conflict with files in same directory
        auto conflict_query = directory_id.has_value()
                                  ? MakeDoc("owner_id", owner_id, "name", new_name, "id", MakeDoc("$ne", file_id), "directory_id", *directory_id)
                                  : MakeDoc("owner_id", owner_id, "name", new_name, "id", MakeDoc("$ne", file_id), "directory_id", MakeDoc("$exists", false));

        auto existing = collection.FindOne(conflict_query);
        if (existing) {
            return Common::Utils::unexpected(UpdateFileError::NameConflict);
        }

        auto now = userver::utils::datetime::Now();

        // Update the file
        auto update_doc = MakeDoc(
            "$set", MakeDoc(
                        "name", new_name,
                        "mime_type", GetMimeType(new_name),
                        "updated_at", now));

        collection.UpdateOne(
            MakeDoc("id", file_id, "owner_id", owner_id),
            update_doc);

        Models::File updated_file{
            .id = file_id,
            .name = new_name,
            .size = current_doc["size"].As<std::int64_t>(),
            .mime_type = GetMimeType(new_name),
            .directory_id = directory_id,
            .owner_id = owner_id,
            .created_at = userver::utils::datetime::TimePointTz{current_doc["created_at"].As<std::chrono::system_clock::time_point>()},
            .updated_at = userver::utils::datetime::TimePointTz{now},
            .status = StringToFileStatus(current_doc["status"].As<std::string>())};

        return updated_file;
    } catch (const userver::storages::mongo::MongoException &e) {
        LOG_ERROR() << "MongoDB error in UpdateFile: " << e.what();
        return Common::Utils::unexpected(UpdateFileError::ServerError);
    } catch (const std::exception &e) {
        LOG_ERROR() << "Error in UpdateFile: " << e.what();
        return Common::Utils::unexpected(UpdateFileError::ServerError);
    }
}

DeleteFileResult MongoFileRepository::DeleteFile(
    const std::string &file_id,
    const std::string &owner_id) {

    try {
        auto collection = pool_->GetCollection(collection_name_);

        using userver::formats::bson::MakeDoc;

        // Check if file exists and belongs to owner
        auto doc_opt = collection.FindOne(MakeDoc("id", file_id, "owner_id", owner_id));
        if (!doc_opt) {
            return Common::Utils::unexpected(DeleteFileError::FileNotFound);
        }

        // Delete the file
        collection.DeleteOne(MakeDoc("id", file_id, "owner_id", owner_id));

        return true;
    } catch (const userver::storages::mongo::MongoException &e) {
        LOG_ERROR() << "MongoDB error in DeleteFile: " << e.what();
        return Common::Utils::unexpected(DeleteFileError::ServerError);
    } catch (const std::exception &e) {
        LOG_ERROR() << "Error in DeleteFile: " << e.what();
        return Common::Utils::unexpected(DeleteFileError::ServerError);
    }
}

void MongoFileRepository::DeleteAll() {
    try {
        auto collection = pool_->GetCollection(collection_name_);
        collection.DeleteMany(userver::formats::bson::MakeDoc());
        LOG_INFO() << "All files deleted from MongoDB";
    } catch (const userver::storages::mongo::MongoException &e) {
        LOG_ERROR() << "MongoDB error in DeleteAll: " << e.what();
        throw;
    }
}

} // namespace Repositories
