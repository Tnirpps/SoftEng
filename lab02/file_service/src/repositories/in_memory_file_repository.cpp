#include "in_memory_file_repository.hpp"

#include <userver/logging/log.hpp>
#include <userver/utils/datetime.hpp>

#include "utils/uuid_generator.hpp"

namespace Repositories {

namespace {
std::string GetMimeType(const std::string &name) {
    // Simple mime type detection based on file extension
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

} // namespace

CreateFileResult InMemoryFileRepository::CreateFile(
    const std::string &name,
    const std::string &content,
    const std::optional<std::string> &directory_id,
    const std::string &owner_id) {

    auto data_ptr = files_.Lock();

    // Check if file with same name and directory already exists
    for (const auto &[id, file] : *data_ptr) {
        if (file.owner_id == owner_id &&
            file.name == name &&
            file.directory_id == directory_id) {
            return std::unexpected(CreateFileError::FileAlreadyExists);
        }
    }

    std::string new_id = Utils::GenerateUuid();
    auto now = userver::utils::datetime::Now();

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

    data_ptr->emplace(new_id, new_file);

    return new_file;
}

GetFileResult InMemoryFileRepository::GetFile(const std::string &file_id) {
    auto data_ptr = files_.Lock();
    auto it = data_ptr->find(file_id);
    if (it == data_ptr->end()) {
        return std::unexpected(GetFileError::FileNotFound);
    }
    return it->second;
}

UpdateFileResult InMemoryFileRepository::UpdateFile(
    const std::string &file_id,
    const std::string &new_name,
    const std::string &owner_id) {

    auto data_ptr = files_.Lock();
    auto it = data_ptr->find(file_id);
    if (it == data_ptr->end()) {
        return std::unexpected(UpdateFileError::FileNotFound);
    }

    if (it->second.owner_id != owner_id) {
        return std::unexpected(UpdateFileError::FileNotFound);
    }

    // Check for name conflict with files in same directory
    for (const auto &[id, file] : *data_ptr) {
        if (id != file_id &&
            file.owner_id == owner_id &&
            file.name == new_name &&
            file.directory_id == it->second.directory_id) {
            return std::unexpected(UpdateFileError::NameConflict);
        }
    }

    it->second.name = new_name;
    it->second.mime_type = GetMimeType(new_name);
    it->second.updated_at = userver::utils::datetime::TimePointTz{userver::utils::datetime::Now()};

    return it->second;
}

DeleteFileResult InMemoryFileRepository::DeleteFile(
    const std::string &file_id,
    const std::string &owner_id) {

    auto data_ptr = files_.Lock();
    auto it = data_ptr->find(file_id);
    if (it == data_ptr->end()) {
        return std::unexpected(DeleteFileError::FileNotFound);
    }

    if (it->second.owner_id != owner_id) {
        return std::unexpected(DeleteFileError::FileNotFound);
    }

    data_ptr->erase(it);

    return true;
}

void InMemoryFileRepository::DeleteAll() {
    auto file_ptr = files_.Lock();
    file_ptr->clear();
}

} // namespace Repositories
