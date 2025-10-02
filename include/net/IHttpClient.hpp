#pragma once
#include <string>
#include <map>

namespace app::net {

struct Response {
    long status{0};
    std::string body;
};

class IHttpClient {
public:
    virtual ~IHttpClient() = default;

    virtual Response postForm(
        const std::string& url,
        const std::map<std::string, std::string>& fields,
        const std::map<std::string, std::string>& headers = {} ) = 0;

    virtual Response get(
        const std::string& url,
        const std::map<std::string, std::string>& headers = {} ) = 0;

    virtual Response postMultipart(
        const std::string& url,
        const std::string& filePath,
        const std::map<std::string, std::string>& fields = {},
        const std::map<std::string, std::string>& headers = {} ) = 0;
};

} // namespace app::net
