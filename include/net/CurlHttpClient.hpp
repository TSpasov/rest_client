#pragma once
#include "IHttpClient.hpp"
#include <curl/curl.h> 

namespace app::net {

class CurlHttpClient : public IHttpClient {
public:
    CurlHttpClient();
    ~CurlHttpClient() override;

    Response postForm(
        const std::string& url,
        const std::map<std::string, std::string>& fields,
        const std::map<std::string, std::string>& headers = {} ) override;

    Response get(
        const std::string& url,
        const std::map<std::string, std::string>& headers = {} ) override;

    Response postMultipart(
        const std::string& url,
        const std::string& filePath,
        const std::map<std::string, std::string>& fields = {},
        const std::map<std::string, std::string>& headers = {} ) override;

private:
    // disallow copying
    CurlHttpClient(const CurlHttpClient&) = delete;
    CurlHttpClient& operator=(const CurlHttpClient&) = delete;
};

} // namespace app::net
