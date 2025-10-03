#pragma once
#include <string>

namespace app::http {

    enum StatusCode {
        HTTP_OK = 200,
        HTTP_CREATED = 201,
        HTTP_NO_CONTENT = 204,

        HTTP_BAD_REQUEST = 400,
        HTTP_UNAUTHORIZED = 401,
        HTTP_FORBIDDEN = 403,
        HTTP_NOT_FOUND = 404,
        HTTP_CONFLICT = 409,

        HTTP_INTERNAL_ERROR = 500,
        HTTP_BAD_GATEWAY = 502,
        HTTP_SERVICE_UNAVAILABLE = 503,
    };

    enum AppError {
        ERR_NONE = 0,
        ERR_AUTH_FAILED = 1001,
        ERR_TOKEN_EXPIRED = 1002,
        ERR_FILE_EXISTS = 1003,
        ERR_UPLOAD_FAILED = 1004,
        ERR_NETWORK = 1005,
        ERR_UNKNOWN = 1099
    };

    enum class Method {
        GET,
        POST,
        PUT,
        PATCH,
        DEL
    };

    inline std::string to_string(Method m) {
        switch (m) {
        case Method::GET:     return "GET";
        case Method::POST:    return "POST";
        case Method::PUT:     return "PUT";
        case Method::PATCH:   return "PATCH";
        case Method::DEL:  return "DELETE";
        default:              return "GET";
        }
    }

    namespace header {
        inline constexpr const char* AUTHORIZATION = "Authorization";
        inline constexpr const char* BEARER_PREFIX = "Bearer ";
        inline constexpr const char* ACCEPT = "Accept";
        inline constexpr const char* JSON = "application/json";
        inline constexpr const char* CONTENT_TYPE = "Content-Type";
        inline constexpr const char* FORM_URLENCODED = "application/x-www-form-urlencoded";
    }

    namespace path {
        inline constexpr const char* TOKEN = "/api/v1/token";
        inline constexpr const char* SELF = "/api/v1/users/self";
        inline constexpr const char* FOLDERS = "/api/v1/folders/"; // append {id}/files
    }

    namespace msg {
        inline constexpr const char* FILE_EXISTS = "File already exists";
        inline constexpr const char* AUTH_DENIED = "Authorization has been denied for this request.";
    }

} // namespace app::http
