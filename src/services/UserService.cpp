#include "services/UserService.hpp"
#include "common/HttpCommon.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace app::services {

    using namespace app::models;
    using namespace app::http;

    User UserService::getSelf(const std::string& host, const Token& token) {
        std::string url = host + path::SELF;

        auto resp = http_.request(
            Method::GET,
            url,
            {
                {header::AUTHORIZATION, std::string(header::BEARER_PREFIX) + token.access_token},
                {header::ACCEPT, header::JSON}
            }
        );

        if (resp.status != HTTP_OK) {
            throw std::runtime_error("GET /users/self failed (" +
                std::to_string(resp.status) + "): " + resp.body);
        }

        auto j = json::parse(resp.body);
        User me;
        me.homeFolderId = j.at("homeFolderID").get<long long>();
        me.username = j.value("username", "");
        me.fullName = j.value("fullName", "");
        return me;
    }

} // namespace app::services
