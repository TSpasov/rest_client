#pragma once
#include <functional>
#include "net/IHttpClient.hpp"
#include "services/AuthService.hpp"
#include "models/Token.hpp"

namespace app::net {

    class RetryingHttpClient : public IHttpClient {
    public:
        RetryingHttpClient(IHttpClient& inner,
            app::services::AuthService& auth,
            std::string host,
            app::models::Token& token)
            : inner_(inner), auth_(auth), host_(std::move(host)), token_(token) {
        }

        HttpResponse request(
            app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& headers = {},
            const std::string& body = "") override;

        HttpResponse requestForm(
            app::http::Method method,
            const std::string& url,
            const std::map<std::string, std::string>& fields,
            const std::map<std::string, std::string>& headers = {}) override;

        HttpResponse requestMultipart(
            app::http::Method method,
            const std::string& url,
            const std::string& filePath,
            const std::map<std::string, std::string>& fields = {},
            const std::map<std::string, std::string>& headers = {}) override;

    private:
        HttpResponse withRetry(std::function<HttpResponse()> fn);

        IHttpClient& inner_;
        app::services::AuthService& auth_;
        std::string host_;
        app::models::Token& token_;
    };

} // namespace app::net
