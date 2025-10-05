#include "common/Logger.hpp"
#include "parser.hpp"
#include "net/CurlHttpClient.hpp"
#include "net/RetryingHttpClient.hpp"
#include "services/AuthService.hpp"
#include "services/UserService.hpp"
#include "services/UploadService.hpp"

using app::common::Logger;
static Logger logger;   // global logger shared across this translation unit

int main(int argc, char* argv[]) {
    logger.info("MOVEit CLI client starting...");

    auto parsed = app::cli::parse(argc, argv);
    if (!parsed.ok()) {
        logger.error(parsed.error->message);
        return parsed.error->exit_code;
    }

    const auto& args = parsed.args;
    const std::string host = args.at(app::cli::ArgType::Host);
    const std::string user = args.at(app::cli::ArgType::User);
    const std::string pass = args.at(app::cli::ArgType::Pass);
    const std::string file = args.at(app::cli::ArgType::FilePath);

    try {
        app::net::CurlHttpClient curl;
        app::services::AuthService auth(curl, user, pass);
        auto token = auth.authenticate(host);
        logger.info("Token acquired (expires in " + std::to_string(token.expires_in) + "s)");

        app::net::RetryingHttpClient http(curl, auth, host, token);
        app::services::UserService users(http);
        app::services::UploadService uploader(http);

        auto me = users.getSelf(host, token);
        logger.info("homeFolderID = " + me.homeFolderId);

        auto result = uploader.uploadFile(host, token, me.homeFolderId, file, false);
        if (!result.success && result.appError == app::http::ERR_FILE_EXISTS) {
            logger.warn("File exists on server: " + file);
            std::cout << "Overwrite? (y/n): ";
            char c; std::cin >> c;
            if (c == 'y' || c == 'Y') {
                if (auto existing = uploader.findFileId(host, token, me.homeFolderId, file)) {
                    if (uploader.deleteFile(host, token, existing.value())) {
                        result = uploader.uploadFile(host, token, me.homeFolderId, file, false);
                    }
                }
                else {
                    logger.warn("Could not locate existing file for overwrite.");
                }
            }
        }

        if (result.success)
            logger.info("Upload OK: " + result.fileName + " (id=" + std::to_string(result.fileId) + ")");
        else
            logger.error("Upload failed: " + result.message);

    }
    catch (const std::exception& ex) {
        logger.error(std::string("Fatal error: ") + ex.what());
        return 99;
    }

    return 0;
}
