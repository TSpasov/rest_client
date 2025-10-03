#pragma once
#include <string>

namespace app::models {

    struct Token {
        std::string access_token;
        std::string refresh_token;
        int expires_in{ 0 };
    };

} // namespace app::models
