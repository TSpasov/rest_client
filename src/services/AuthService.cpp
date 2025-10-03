#include "services/AuthService.hpp"
#include "common/HttpCommon.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace app::services {

    using namespace app::models;
    using namespace app::http;

    Token AuthService::authenticate(const std::string& host) {
        std::string url = host + path::TOKEN;

        auto resp = http_.requestForm(
            Method::POST,
            url,
            { {"grant_type", "password"},
              {"username", user_},
             {"password", pass_} },
            { {header::CONTENT_TYPE, header::FORM_URLENCODED} }
        );

        if (resp.status != HTTP_OK) {
            throw std::runtime_error("Auth failed (" + std::to_string(resp.status) + "): " + resp.body);
        }

        auto j = json::parse(resp.body);
        Token token;
        token.access_token = j.at("access_token").get<std::string>();
        token.refresh_token = j.value("refresh_token", "");
        token.expires_in = j.value("expires_in", 0);
        return token;
    }

    Token AuthService::refresh(const std::string& host, const Token& old) {
        std::string url = host + path::TOKEN;

        auto resp = http_.requestForm(
            Method::POST,
            url,
            { {"grant_type", "refresh_token"},
             {"refresh_token", old.refresh_token} },
            { {header::CONTENT_TYPE, header::FORM_URLENCODED} }
        );

        if (resp.status != HTTP_OK) {
            return authenticate(host);
        }

        auto j = json::parse(resp.body);
        Token token;
        token.access_token = j.at("access_token").get<std::string>();
        token.refresh_token = j.value("refresh_token", "");
        token.expires_in = j.value("expires_in", 0);
        return token;
    }

} // namespace app::services
