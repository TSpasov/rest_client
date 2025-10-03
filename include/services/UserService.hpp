#pragma once
#include <string>
#include "net/IHttpClient.hpp"
#include "models/User.hpp"
#include "models/Token.hpp"

namespace app::services {

    class UserService {
    public:
        explicit UserService(app::net::IHttpClient& http) : http_(http) {}

        app::models::User getSelf(const std::string& host, const app::models::Token& token);

    private:
        app::net::IHttpClient& http_;
    };

} // namespace app::services
