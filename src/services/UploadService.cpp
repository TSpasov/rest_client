#include "services/UploadService.hpp"
#include "common/HttpCommon.hpp"
#include "json.hpp"

#include <iostream>
#include <filesystem>


using json = nlohmann::json;

namespace app::services {

    using namespace app::models;
    using namespace app::http;
    using namespace app::net;

    UploadResult UploadService::uploadFile(const std::string& host,
        const Token& token,
        const std::string& folderId,
        const std::string& filePath,
        bool overwrite) {
        std::string url = host + path::FOLDERS + folderId + "/files";

        std::map<std::string, std::string> headers = {
            {header::AUTHORIZATION, std::string(header::BEARER_PREFIX) + token.access_token}
        };

        HttpResponse resp;
        resp = http_.requestMultipart(Method::POST, url, filePath, {}, headers);
       

        UploadResult result;
        result.success = (resp.status == HTTP_CREATED);

        if (!result.success) {
            if (resp.status == HTTP_CONFLICT) {
                result.message = msg::FILE_EXISTS;
                result.appError = ERR_FILE_EXISTS;
            }
            else {
                result.message = resp.body;
                result.appError = ERR_UPLOAD_FAILED;
            }
            return result;
        }

        auto j = json::parse(resp.body);

        result.fileName = j.value("name", "");
        result.fileId = std::stoll(j["id"].get<std::string>());
        result.message = "OK";
        result.appError = ERR_NONE;
        return result;
    }


    std::optional<long long> services::UploadService::findFileId(
        const std::string& host,
        const models::Token& token,
        const std::string& folderId,
        const std::string& fileName)
    {

        namespace fs = std::filesystem;
        std::string baseName = fs::path(fileName).filename().string();

        std::string url = host + "/api/v1/folders/" + folderId + "/files";
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + token.access_token},
            {"Accept", "application/json"}
        };

        auto resp = http_.request(http::Method::GET, url, headers, {});
        if (resp.status != http::HTTP_OK) {
            std::cerr << "Failed to list files: " << resp.body << "\n";
            return std::nullopt;
        }

        try {

            json j = json::parse(resp.body);

            if (j.contains("items") && j["items"].is_array()) {
                for (const auto& item : j["items"]) {
                    if (item.contains("name") && item["name"] == baseName) {
                        return std::stoll(item["id"].get<std::string>());
                    }
                }
            }
            else {
                std::cerr << "Unexpected JSON format: no 'items' array found\n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "JSON parse error in findFileId: " << e.what() << "\n";
        }

        return std::nullopt;
    }

    bool services::UploadService::deleteFile(
        const std::string& host,
        const models::Token& token,
        long long fileId)
    {
        std::string url = host + "/api/v1/files/" + std::to_string(fileId);
        std::map<std::string, std::string> headers = {
            {"Authorization", "Bearer " + token.access_token}
        };

        auto resp = http_.request(http::Method::DEL, url, headers, {});

        if (resp.status == http::HTTP_NO_CONTENT) {
            std::cout << " File deleted successfully (id=" << fileId << ")\n";
            return true;
        }

        std::cerr << "Delete failed (" << resp.status << "): " << resp.body << "\n";
        return false;
    }

} // namespace app::services
