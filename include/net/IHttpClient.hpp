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

        virtual HttpResponse request(app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "") = 0;

        virtual HttpResponse requestForm(app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& fields,
            const std::map<std::string, std::string>& headers = {}) = 0;

        virtual HttpResponse requestMultipart(app::http::Method method,
            const std::string& url,
            const std::string& filePath,
            const std::map<std::string, std::string>& fields = {},
            const std::map<std::string, std::string>& headers = {}) = 0;
    };

} // namespace app::net
