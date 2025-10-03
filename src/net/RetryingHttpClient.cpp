#include "net/RetryingHttpClient.hpp"

namespace app::net {

    HttpResponse RetryingHttpClient::withRetry(std::function<HttpResponse()> fn) {
        auto resp = fn();
        if (resp.status == 401) {
            token_ = auth_.refresh(host_, token_);
            resp = fn();
        }
        return resp;
    }

    HttpResponse RetryingHttpClient::request(app::http::Method method,
        const std::string& url,
        const std::map<std::string, std::string>& headers,
        const std::string& body)
    {
        return withRetry([&]() {
            auto hdrs = headers;
            hdrs["Authorization"] = "Bearer " + token_.access_token;
            return inner_.request(method, url, hdrs, body);
            });
    }

    HttpResponse RetryingHttpClient::requestForm(app::http::Method method,
        const std::string& url,
        const std::map<std::string, std::string>& fields,
        const std::map<std::string, std::string>& headers) {
        return withRetry([&]() {
            auto hdrs = headers;
            hdrs["Authorization"] = "Bearer " + token_.access_token;
            return inner_.requestForm(method, url, fields, hdrs);
            });
    }

    HttpResponse RetryingHttpClient::requestMultipart(app::http::Method method,
        const std::string& url,
        const std::string& filePath,
        const std::map<std::string, std::string>& fields,
        const std::map<std::string, std::string>& headers) {
        return withRetry([&]() {
            auto hdrs = headers;
            hdrs["Authorization"] = "Bearer " + token_.access_token;
            return inner_.requestMultipart(method, url, filePath, fields, hdrs);
            });
    }
} // namespace app::net
