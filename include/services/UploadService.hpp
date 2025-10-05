#pragma once
#include "net/IHttpClient.hpp"
#include "models/Token.hpp"
#include "models/UploadResult.hpp"
#include "common/HttpCommon.hpp"
#include <string>
#include <optional>

namespace app::services {

    class UploadService {
    public:
        explicit UploadService(app::net::IHttpClient& http) : http_(http) {}

        app::models::UploadResult uploadFile(
            const std::string& host,
            const app::models::Token& token,
            const std::string& folderId,
            const std::string& filePath,
            bool overwrite);

        std::optional<long long> findFileId(
            const std::string& host,
            const app::models::Token& token,
            const std::string& folderId,
            const std::string& fileName);

        bool deleteFile(
            const std::string& host,
            const app::models::Token& token,
            long long fileId);

    private:
        app::net::IHttpClient& http_;
    };

} // namespace app::services
