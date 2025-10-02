#pragma once
#include <string>
#include <map>
#include <optional>

namespace app::cli {

enum class ArgType {
    Host,
    User,
    Pass,
    FilePath
};

struct ParseError {
    int exit_code;
    std::string message;
};

struct ParseResult {
    std::map<ArgType, std::string> args;   // always available
    std::optional<ParseError> error;       // error details if parsing failed

    bool ok() const { return !error.has_value(); }
};

inline ParseResult parse(int argc, char** argv) {
    ParseResult res;

    if (argc < 5) {
        res.error = ParseError{
            1,
            std::string("Usage: ") + argv[0] + " <host> <username> <password> <file>"
        };
        return res;
    }

    res.args[ArgType::Host]     = argv[1];
    res.args[ArgType::User]     = argv[2];
    res.args[ArgType::Pass]     = argv[3];
    res.args[ArgType::FilePath] = argv[4];

    return res;
}

} // namespace app::cli
