#pragma once
#include <string>
#include "net/IHttpClient.hpp"
#include "models/Token.hpp"
#include "models/UploadResult.hpp"

namespace app::services {

    class UploadService {
    public:
        explicit UploadService(app::net::IHttpClient& http) : http_(http) {}

        app::models::UploadResult uploadFile(const std::string& host,
            const app::models::Token& token,
            long long folderId,
            const std::string& filePath,
            bool overwrite = false);

    private:
        app::net::IHttpClient& http_;
    };

} // namespace app::services
