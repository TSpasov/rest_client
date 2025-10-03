#pragma once
#include <string>
#include "net/IHttpClient.hpp"
#include "models/Token.hpp"

namespace app::services {

    class AuthService {
    public:
        AuthService(app::net::IHttpClient& http,
            std::string user,
            std::string pass)
            : http_(http), user_(std::move(user)), pass_(std::move(pass)) {
        }

        app::models::Token authenticate(const std::string& host);
        app::models::Token refresh(const std::string& host, const app::models::Token& old);

    private:
        app::net::IHttpClient& http_;
        std::string user_;
        std::string pass_;
    };

} // namespace app::services
