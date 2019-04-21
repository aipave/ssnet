#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <cstring>

#include <unistd.h>

namespace ssnet {

class Logger {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        EnumSize
    };

    Logger(LogLevel level, const std::string &srcFile, uint32_t line, const std::string &functionName) :
            _level(level),
            _fileInfo(srcFile + ":" + std::to_string(line)),
            _functionName(functionName) {

        time_t now = time(nullptr);
        char buffer[20]; // YYYY-MM-DD HH:MM:SS
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        _ss << "[" << buffer << " - " << getpid() << " - ";
        const char *levelName[LogLevel::EnumSize] =
                {
                        "DEBUG",
                        "INFO",
                        "WARN",
                        "ERROR",
                        "FATAL"
                };
        _ss << levelName[level] << " - " << _fileInfo << " " << _functionName << "()] ";
    }

    ~Logger() {
        if (errno != 0) {
            char buf[256];
            if (strerror_r(errno, buf, sizeof(buf)) == 0) {
                _ss << " - errno: " << errno << " - " << buf;
            } else {
                // TODO Handle the error in case strerror_r fails
            }
            errno = 0; // Reset errno
        }

        if (_level >= logLevel()) {
            _ss << "\n";
            std::cout << _ss.str();
        }
        if (_level > LogLevel::FATAL) {
            abort();
        }
    }

    static LogLevel logLevel();

    static void setLogLevel(LogLevel level);

    std::stringstream &stream() { return _ss; }

private:
    LogLevel _level;
    std::string _fileInfo;
    std::string _functionName;
    std::stringstream _ss;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() { return g_logLevel; }

inline void Logger::setLogLevel(Logger::LogLevel level) { g_logLevel = level; }

} // namespace ssnet


#define LOG(level) \
    if (ssnet::Logger::LogLevel::level >= ssnet::Logger::logLevel()) \
        ssnet::Logger(ssnet::Logger::LogLevel::level, __FILE__, __LINE__, __FUNCTION__).stream()

// TODO more interface