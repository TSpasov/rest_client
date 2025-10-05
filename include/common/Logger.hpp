#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <windows.h>

namespace app::common {

    enum class OutputTarget {
        Console,
        File,
        Gui
    };

    class Logger {
        std::mutex mtx_;
        OutputTarget target_ = OutputTarget::Console;
        std::ofstream file_;
        HWND hwnd_ = nullptr;

    public:
        // Default: console output (same behavior as before)
        explicit Logger(OutputTarget target = OutputTarget::Console,
            const std::string& filePath = "",
            HWND hwnd = nullptr)
            : target_(target), hwnd_(hwnd)
        {
            if (target_ == OutputTarget::File && !filePath.empty()) {
                file_.open(filePath, std::ios::app);
            }
        }

        void setGuiTarget(HWND hwnd) { hwnd_ = hwnd; target_ = OutputTarget::Gui; }
        void info(const std::string& msg) { write("INFO", msg, std::cout); }
        void warn(const std::string& msg) { write("WARN", msg, std::cout); }
        void error(const std::string& msg) { write("ERROR", msg, std::cerr); }

    private:
        void write(const std::string& level, const std::string& msg, std::ostream& stream) {
            std::lock_guard<std::mutex> lock(mtx_);
            std::string line = timestamp() + " [" + level + "] " + msg + "\n";

            switch (target_) {
            case OutputTarget::Console:
                stream << line;
                break;

            case OutputTarget::File:
                if (file_.is_open()) file_ << line;
                break;

            case OutputTarget::Gui:
                if (hwnd_) {
                    SendMessageA(hwnd_, EM_SETSEL, -1, -1);
                    SendMessageA(hwnd_, EM_REPLACESEL, FALSE, (LPARAM)line.c_str());
                }
                break;
            }

            OutputDebugStringA(line.c_str()); // still visible in Visual Studio output window
        }

        std::string timestamp() const {
            auto now = std::chrono::system_clock::now();
            auto t = std::chrono::system_clock::to_time_t(now);
            std::tm tm{};
            localtime_s(&tm, &t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%H:%M:%S");
            return oss.str();
        }
    };

} // namespace app::common
