
#include <curl/curl.h>
#include "parser.hpp"   // provides app::cli::parse + ArgType map
#include "json.hpp"     // nlohmann::json (header-only)
#include <iostream>
#include <fstream>
#include <map>
#include <string>

using json = nlohmann::json;

//  TODO next steps:
// * Use header-only json.hpp for proper parsing
// * Add a tiny HttpClient wrapper (RAII)
// * Improve error handling
// * Add retries on network failures
// * Add progress bar for uploads
// * move to OOP services (Auth/User/Upload) using IHttpClient

namespace {

    size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t total = size * nmemb;
        std::string* s = static_cast<std::string*>(userp);
        s->append(static_cast<char*>(contents), total);
        return total;
    }

    long post_form(const std::string& url,
        const std::map<std::string, std::string>& fields,
        std::string& outBody) {
        CURL* curl = curl_easy_init();
        if (!curl) return 0;

        std::string data;
        bool first = true;
        for (auto& kv : fields) {
            if (!first) data.push_back('&'); first = false;
            char* k = curl_easy_escape(curl, kv.first.c_str(), (int)kv.first.size());
            char* v = curl_easy_escape(curl, kv.second.c_str(), (int)kv.second.size());
            data += k; data += '='; data += v;
            curl_free(k); curl_free(v);
        }

        struct curl_slist* hdrs = nullptr;
        hdrs = curl_slist_append(hdrs, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outBody);

        CURLcode rc = curl_easy_perform(curl);
        long http = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http);
        if (rc != CURLE_OK && http == 0) outBody = std::string("curl error: ") + curl_easy_strerror(rc);

        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
        return http;
    }

    long get_bearer(const std::string& url,
        const std::string& token,
        std::string& outBody) {
        CURL* curl = curl_easy_init();
        if (!curl) return 0;

        struct curl_slist* hdrs = nullptr;
        std::string auth = "Authorization: Bearer " + token;
        hdrs = curl_slist_append(hdrs, auth.c_str());
        hdrs = curl_slist_append(hdrs, "Accept: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outBody);

        CURLcode rc = curl_easy_perform(curl);
        long http = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http);
        if (rc != CURLE_OK && http == 0) outBody = std::string("curl error: ") + curl_easy_strerror(rc);

        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
        return http;
    }

    long post_multipart_file(const std::string& url,
        const std::string& token,
        const std::string& filePath,
        std::string& outBody) {
        CURL* curl = curl_easy_init();
        if (!curl) return 0;

        struct curl_slist* hdrs = nullptr;
        std::string auth = "Authorization: Bearer " + token;
        hdrs = curl_slist_append(hdrs, auth.c_str());

        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filedata(part, filePath.c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outBody);

        CURLcode rc = curl_easy_perform(curl);
        long http = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http);
        if (rc != CURLE_OK && http == 0) outBody = std::string("curl error: ") + curl_easy_strerror(rc);

        curl_mime_free(mime);
        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
        return http;
    }

} // namespace

int main(int argc, char* argv[]) {
    auto parsed = app::cli::parse(argc, argv);
    if (!parsed.ok()) {
        std::cerr << parsed.error->message << "\n";
        return parsed.error->exit_code;
    }

    const auto& args = parsed.args;
    const std::string host = args.at(app::cli::ArgType::Host);
    const std::string user = args.at(app::cli::ArgType::User);
    const std::string pass = args.at(app::cli::ArgType::Pass);
    const std::string file = args.at(app::cli::ArgType::FilePath);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::string tokenResp;
    long http = post_form(host + "/api/v1/token",
        { {"grant_type","password"}, {"username", user}, {"password", pass} },
        tokenResp);
    if (http != 200) {
        std::cerr << "Auth failed (" << http << "): " << tokenResp << "\n";
        curl_global_cleanup();
        return 2;
    }

    std::string access_token;
    try {
        access_token = json::parse(tokenResp).at("access_token").get<std::string>();
    }
    catch (...) {
        std::cerr << "Token parse error\n" << tokenResp << "\n";
        curl_global_cleanup();
        return 3;
    }
    std::cout << "Token OK\n";

    std::string selfResp;
    http = get_bearer(host + "/api/v1/users/self", access_token, selfResp);
    if (http != 200) {
        std::cerr << "GET /users/self failed (" << http << "): " << selfResp << "\n";
        curl_global_cleanup();
        return 4;
    }

    long long homeId = 0;
    try {
        homeId = json::parse(selfResp).at("homeFolderID").get<long long>();
    }
    catch (...) {
        std::cerr << "Self parse error\n" << selfResp << "\n";
        curl_global_cleanup();
        return 5;
    }
    std::cout << "homeFolderID = " << homeId << "\n";

    std::ifstream f(file, std::ios::binary);
    if (!f) {
        std::cerr << "File not found: " << file << "\n";
        curl_global_cleanup();
        return 6;
    }

    std::string upResp;
    http = post_multipart_file(host + "/api/v1/folders/" + std::to_string(homeId) + "/files",
        access_token, file, upResp);
    if (http != 201) {
        std::cerr << "Upload failed (" << http << "): " << upResp << "\n";
        curl_global_cleanup();
        return 7;
    }

    std::cout << "Upload OK\n" << upResp << "\n";

    curl_global_cleanup();
    return 0;
}
