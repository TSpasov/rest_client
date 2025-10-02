#include "parser.hpp"
#include "json.hpp"
#include "net/CurlHttpClient.hpp"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

//  TODO next steps:
// * Use header-only json.hpp for proper parsing
// * Add a tiny HttpClient wrapper (RAII)
// * Improve error handling
// * Add retries on network failures
// * Add progress bar for uploads
// * move to OOP services (Auth/User/Upload) using IHttpClient

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

    app::net::CurlHttpClient http;

    auto tokenResp = http.postForm(host + "/api/v1/token",
        { {"grant_type","password"}, {"username",user}, {"password",pass} });
    if (tokenResp.status != 200) {
        std::cerr << "Auth failed (" << tokenResp.status << "): " << tokenResp.body << "\n";
        return 2;
    }
    std::string access = json::parse(tokenResp.body).at("access_token");
    std::cout << " Token acquired\n";

    auto selfResp = http.get(host + "/api/v1/users/self",
        { {"Authorization","Bearer " + access},
         {"Accept","application/json"} });
    if (selfResp.status != 200) {
        std::cerr << "Self failed (" << selfResp.status << "): " << selfResp.body << "\n";
        return 3;
    }

    long long homeId = json::parse(selfResp.body).at("homeFolderID");
    std::cout << " homeFolderID = " << homeId << "\n";


    std::ifstream f(file, std::ios::binary);
    if (!f) {
        std::cerr << "File not found: " << file << "\n";
        return 4;
    }

    auto upResp = http.postMultipart(host + "/api/v1/folders/" + std::to_string(homeId) + "/files",
        file,
        {},
        { {"Authorization","Bearer " + access} });

    if (upResp.status != 201) {
        std::cerr << "Upload failed (" << upResp.status << "): " << upResp.body << "\n";
        return 5;
    }
    std::cout << " Upload OK\n" << upResp.body << "\n";

    return 0;
}
