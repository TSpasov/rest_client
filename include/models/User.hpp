#pragma once
#include <string>

namespace app::models {

    struct User {
        std::string homeFolderId{};
        std::string username{};
        std::string fullName{};
    };

} // namespace app::models
