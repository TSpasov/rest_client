#pragma once
#include <string>

namespace app::models {

    struct User {
        long long homeFolderId{ 0 };
        std::string username;
        std::string fullName;
    };

} // namespace app::models
