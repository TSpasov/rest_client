#include "net/CurlHttpClient.hpp"
#include <sstream>
#include <stdexcept>

namespace {

// callback for writing response body into std::string
size_t write_cb(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    std::string* out = static_cast<std::string*>(userdata);
    out->append(static_cast<char*>(ptr), total);
    return total;
}

} // namespace

namespace app::net {

CurlHttpClient::CurlHttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

CurlHttpClient::~CurlHttpClient() {
    curl_global_cleanup();
}

Response CurlHttpClient::postForm(
    const std::string& url,
    const std::map<std::string, std::string>& fields,
    const std::map<std::string, std::string>& headers)
{
    Response r;
    CURL* curl = curl_easy_init();
    if (!curl) { r.body = "curl init failed"; return r; }

    // encode fields into x-www-form-urlencoded string
    std::string data;
    bool first = true;

    for (auto& kv : fields) {
        if (!first)
            data.push_back('&');

        first = false;
        char* k = curl_easy_escape(curl, kv.first.c_str(), (int)kv.first.size());
        char* v = curl_easy_escape(curl, kv.second.c_str(), (int)kv.second.size());
        data += k;
        data += '='; 
        data += v;
        curl_free(k);
        curl_free(v);
    }

    struct curl_slist* hdrs = nullptr;
    hdrs = curl_slist_append(hdrs, "Content-Type: application/x-www-form-urlencoded");

    for (auto& kv : headers) {
        hdrs = curl_slist_append(hdrs, (kv.first + ": " + kv.second).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);

    CURLcode rc = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r.status);

    if (rc != CURLE_OK && r.status == 0) {
        r.body = std::string("curl error: ") + curl_easy_strerror(rc);
    }

    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    return r;
}

Response CurlHttpClient::get(
    const std::string& url,
    const std::map<std::string, std::string>& headers)
{
    Response r;
    CURL* curl = curl_easy_init();
    if (!curl) {
    r.body = "curl init failed";
    return r;
    }

    struct curl_slist* hdrs = nullptr;
    for (auto& kv : headers) {
        hdrs = curl_slist_append(hdrs, (kv.first + ": " + kv.second).c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);

    CURLcode rc = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r.status);
    if (rc != CURLE_OK && r.status == 0) {
        r.body = std::string("curl error: ") + curl_easy_strerror(rc);
    }

    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    return r;
}

Response CurlHttpClient::postMultipart(
    const std::string& url,
    const std::string& filePath,
    const std::map<std::string, std::string>& fields,
    const std::map<std::string, std::string>& headers)
{
    Response r;
    CURL* curl = curl_easy_init();
    if (!curl) { r.body = "curl init failed"; return r; }

    struct curl_slist* hdrs = nullptr;
    for (auto& kv : headers) {
        hdrs = curl_slist_append(hdrs, (kv.first + ": " + kv.second).c_str());
    }

    curl_mime* mime = curl_mime_init(curl);

    // file part
    curl_mimepart* part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, filePath.c_str());

    // extra text fields
    for (auto& kv : fields) {
        curl_mimepart* p = curl_mime_addpart(mime);
        curl_mime_name(p, kv.first.c_str());
        curl_mime_data(p, kv.second.c_str(), CURL_ZERO_TERMINATED);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);

    CURLcode rc = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r.status);
    if (rc != CURLE_OK && r.status == 0) {
        r.body = std::string("curl error: ") + curl_easy_strerror(rc);
    }

    curl_mime_free(mime);
    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    return r;
}

} // namespace app::net
