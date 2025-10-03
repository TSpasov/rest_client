#include "services/UploadService.hpp"
#include "common/HttpCommon.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace app::services {

    using namespace app::models;
    using namespace app::http;
    using namespace app::net;

    UploadResult UploadService::uploadFile(const std::string& host,
        const Token& token,
        long long folderId,
        const std::string& filePath,
        bool overwrite) {
        std::string url = host + path::FOLDERS + std::to_string(folderId) + "/files";

        std::map<std::string, std::string> headers = {
            {header::AUTHORIZATION, std::string(header::BEARER_PREFIX) + token.access_token}
        };

        HttpResponse resp;

        if (overwrite) {
            // Try PATCH if overwriting
            resp = http_.requestMultipart(Method::PATCH, url, filePath, {}, headers);
        }
        else {
            resp = http_.requestMultipart(Method::POST, url, filePath, {}, headers);
        }

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
        result.fileId = j.value("id", 0);
        result.message = "OK";
        result.appError = ERR_NONE;
        return result;
    }

} // namespace app::services
