#pragma once
#include "net/IHttpClient.hpp"
#include <curl/curl.h>

namespace app::net {

    class CurlHttpClient : public IHttpClient {
    public:
        CurlHttpClient();
        ~CurlHttpClient();

        HttpResponse request(app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "") override;

        HttpResponse requestForm(app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& fields,
            const std::map<std::string, std::string>& headers = {}) override;

        HttpResponse requestMultipart(app::http::Method method,
            const std::string& url,
            const std::string& filePath,
            const std::map<std::string, std::string>& fields = {},
            const std::map<std::string, std::string>& headers = {}) override;

    private:
        void applyMethod(CURL* curl, app::http::Method method);
    };
} // namespace app::net
