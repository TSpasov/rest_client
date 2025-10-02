#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <string>


//  TODO next steps:
// * Use header-only json.hpp for proper parsing
// * Add a tiny HttpClient wrapper (RAII)
// * Improve error handling
// * Add retries on network failures
// * Add progress bar for uploads


namespace {

    size_t write_cb(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    std::string* out = static_cast<std::string*>(userdata);
    out->append(static_cast<char*>(ptr), total);
    return total;
}

std::string json_get_string(const std::string& s, const std::string& key) {
    std::string pat = "\"" + key + "\":\"";
    size_t p = s.find(pat);
    if (p == std::string::npos) return "";
    p += pat.size();
    size_t q = s.find('"', p);
    if (q == std::string::npos) return "";
    return s.substr(p, q - p);
}

std::string json_get_number(const std::string& s, const std::string& key) {
    std::string pat = "\"" + key + "\":";
    size_t p = s.find(pat);
    if (p == std::string::npos) return "";
    p += pat.size();
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) ++p;
    size_t q = p;
    while (q < s.size() && (std::isdigit(static_cast<unsigned char>(s[q])) || s[q] == '-')) ++q;
    return s.substr(p, q - p);
}

}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <host> <username> <password> <file>\n";
        return 1;
    }

    std::string host = argv[1];
    if (!host.empty() && host.back() == '/') host.pop_back();
    std::string user = argv[2];
    std::string pass = argv[3];
    std::string file = argv[4];

    {
        std::ifstream f(file, std::ios::binary);
        if (!f) {
            std::cerr << "file not found: " << file << "\n";
            return 2;
        }
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::string tokenBody;
    long tokenCode = 0;
    {
        CURL* curl = curl_easy_init();
        if (!curl) return 3;

        std::string url = host + "/api/v1/token";

        std::string data;
        {
            char* g = curl_easy_escape(curl, "grant_type", 10);
            char* gp = curl_easy_escape(curl, "password", 8);
            char* u = curl_easy_escape(curl, "username", 8);
            char* ue = curl_easy_escape(curl, user.c_str(), (int)user.size());
            char* p = curl_easy_escape(curl, "password", 8);
            char* pe = curl_easy_escape(curl, pass.c_str(), (int)pass.size());

            data.reserve(64 + user.size() + pass.size());
            data.append(g).append("=").append(gp).append("&")
                .append(u).append("=").append(ue).append("&")
                .append(p).append("=").append(pe);

            curl_free(g); curl_free(gp); curl_free(u); curl_free(ue); curl_free(p); curl_free(pe);
        }

        struct curl_slist* hdrs = nullptr;
        hdrs = curl_slist_append(hdrs, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tokenBody);

        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &tokenCode);

        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
    }

    if (tokenCode != 200) {
        std::cerr << "auth failed http=" << tokenCode << "\n" << tokenBody << "\n";
        curl_global_cleanup();
        return 4;
    }

    std::string access = json_get_string(tokenBody, "access_token");
    if (access.empty()) {
        std::cerr << "could not parse access_token\n" << tokenBody << "\n";
        curl_global_cleanup();
        return 5;
    }
    std::cout << "Token OK\n";

    std::string selfBody;
    long selfCode = 0;
    std::string homeFolderId;
    {
        CURL* curl = curl_easy_init();
        if (!curl) return 6;

        std::string url = host + "/api/v1/users/self";
        struct curl_slist* hdrs = nullptr;
        std::string auth = "Authorization: Bearer " + access;
        hdrs = curl_slist_append(hdrs, auth.c_str());
        hdrs = curl_slist_append(hdrs, "Accept: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &selfBody);

        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &selfCode);

        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
    }

    if (selfCode != 200) {
        std::cerr << "GET /users/self failed http=" << selfCode << "\n" << selfBody << "\n";
        curl_global_cleanup();
        return 7;
    }

    homeFolderId = json_get_number(selfBody, "homeFolderID");
    if (homeFolderId.empty()) {
        std::cerr << "could not parse homeFolderID\n" << selfBody << "\n";
        curl_global_cleanup();
        return 8;
    }
    std::cout << "homeFolderID = " << homeFolderId << "\n";

    std::string upBody;
    long upCode = 0;
    {
        CURL* curl = curl_easy_init();
        if (!curl) return 9;

        std::string url = host + "/api/v1/folders/" + homeFolderId + "/files";
        struct curl_slist* hdrs = nullptr;
        std::string auth = "Authorization: Bearer " + access;
        hdrs = curl_slist_append(hdrs, auth.c_str());

        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filedata(part, file.c_str());

        curl_mimepart* part2 = curl_mime_addpart(mime);
        curl_mime_name(part2, "comments");
        curl_mime_data(part2, "uploaded from minimal test client", CURL_ZERO_TERMINATED);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &upBody);

        CURLcode rc = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &upCode);

        curl_mime_free(mime);
        curl_slist_free_all(hdrs);
        curl_easy_cleanup(curl);
    }

    if (upCode != 201) {
        std::cerr << "upload failed http=" << upCode << "\n" << upBody << "\n";
        curl_global_cleanup();
        return 10;
    }

    std::cout << "Upload OK\n" << upBody << "\n";

    curl_global_cleanup();
    return 0;
}
