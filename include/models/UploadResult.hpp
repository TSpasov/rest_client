#pragma once
#include <string>

namespace app::models {

    struct UploadResult {
        bool success{ false };
        std::string fileName;
        long long fileId{ 0 };
        std::string message;
        int appError{ 0 };
    };

} // namespace app::models
