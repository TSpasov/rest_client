#pragma once
#include <string>
#include <map>
#include "common/HttpCommon.hpp"

namespace app::net {

    struct HttpResponse {
        long status{};
        std::string body;
    };

    struct IHttpClient {
        virtual ~IHttpClient() = default;

        // Generic plain request (GET/DELETE/PUT with optional body)
        virtual HttpResponse request(app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "") = 0;

        // Form-encoded POST/PUT
        virtual HttpResponse requestForm(app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& fields,
            const std::map<std::string, std::string>& headers = {}) = 0;

        // Multipart file upload
        virtual HttpResponse requestMultipart(app::http::Method method,
            const std::string& url,
            const std::string& filePath,
            const std::map<std::string, std::string>& fields = {},
            const std::map<std::string, std::string>& headers = {}) = 0;
    };

} // namespace app::net
