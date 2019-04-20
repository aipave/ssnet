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
            level_(level),
            fileInfo_(srcFile + ":" + std::to_string(line)),
            functionName_(functionName) {

        time_t now = time(nullptr);
        char buffer[20]; // YYYY-MM-DD HH:MM:SS
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        ss_ << "[" << buffer << " - " << getpid() << " - ";
        const char *levelName[LogLevel::EnumSize] =
                {
                        "DEBUG",
                        "INFO",
                        "WARN",
                        "ERROR",
                        "FATAL"
                };
        ss_ << levelName[level] << " - " << fileInfo_ << " " << functionName_ << "()] ";
    }

    ~Logger() {
        if (errno != 0) {
            char buf[256];
            strerror_r(errno, buf, sizeof(buf));
            ss_ << " - errno: " << errno << " - " << buf;
            errno = 0; // reset errno
        }

        if (level_ >= logLevel()) {
            ss_ << "\n" ;
            std::cout << ss_.str();
        }
        if (level_ > LogLevel::FATAL) {
            abort();
        }
    }

    static LogLevel logLevel();

    static void setLogLevel(LogLevel level);

    std::stringstream &stream() { return ss_; }

private:
    LogLevel level_;
    std::string fileInfo_;
    std::string functionName_;
    std::stringstream ss_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() { return g_logLevel; }

inline void Logger::setLogLevel(Logger::LogLevel level) { g_logLevel = level; }

} // namespace ssnet


#define LOG(level) \
    if (ssnet::Logger::LogLevel::level >= ssnet::Logger::logLevel()) \
        ssnet::Logger(ssnet::Logger::LogLevel::level, __FILE__, __LINE__, __FUNCTION__).stream()

// todo 提供条件log语句,如 LogInfoIf(1>3) << "1 > 3";