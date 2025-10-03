#include <iostream>
#include "parser.hpp"

#include "net/CurlHttpClient.hpp"
#include "net/RetryingHttpClient.hpp"

#include "services/AuthService.hpp"
#include "services/UserService.hpp"
#include "services/UploadService.hpp"

#include "models/Token.hpp"
#include "models/User.hpp"
#include "models/UploadResult.hpp"

#include "common/HttpCommon.hpp"

//  TODO:
// * Add progress bar
// * Add logging (instead of std::cout)
// * Add CLI flags for overwrite mode
// * Improve error handling UX

int main(int argc, char* argv[]) {
    auto parsed = app::cli::parse(argc, argv);
    if (!parsed.ok()) {
        std::cerr << parsed.error->message << "\n";
        return parsed.error->exit_code;
    }

    const auto& args = parsed.args;
    std::string host = args.at(app::cli::ArgType::Host);
    std::string user = args.at(app::cli::ArgType::User);
    std::string pass = args.at(app::cli::ArgType::Pass);
    std::string file = args.at(app::cli::ArgType::FilePath);

    try {
        app::net::CurlHttpClient curl;

        app::services::AuthService auth(curl, user, pass);
        app::models::Token token = auth.authenticate(host);
        std::cout << "Token acquired (expires in " << token.expires_in << "s)\n";

        app::net::RetryingHttpClient http(curl, auth, host, token);

        app::services::UserService users(http);
        app::services::UploadService uploader(http);

        app::models::User me = users.getSelf(host, token);
        std::cout << "homeFolderID = " << me.homeFolderId << "\n";

        app::models::UploadResult result =
            uploader.uploadFile(host, token, me.homeFolderId, file, false);

        if (!result.success && result.appError == app::http::ERR_FILE_EXISTS) {
            std::cout << "File exists. Overwrite? (y/n): ";
            char c;
            std::cin >> c;
            if (c == 'y' || c == 'Y') {
                result = uploader.uploadFile(host, token, me.homeFolderId, file, true);
            }
        }

        if (result.success) {
            std::cout << "Upload OK: " << result.fileName << " (id=" << result.fileId << ")\n";
        }
        else {
            std::cerr << "Upload failed: " << result.message << "\n";
            return 2;
        }

    }
    catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 99;
    }

    return 0;
}
