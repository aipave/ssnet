#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <sstream>

#include "Logger.h"

namespace {

void testLogLevelSetting() {
    std::cout << "ssnet::Logger::setLogLevel(ssnet::Logger::LogLevel::INFO)" << std::endl;

    ssnet::Logger::setLogLevel(ssnet::Logger::LogLevel::INFO);

    LOG(DEBUG) << "Debug message";
    LOG(INFO) << "Info message";
    LOG(WARN) << "Warning message";
    LOG(ERROR) << "Error message";
    LOG(FATAL) << "Fatal message";


    std::cout << "ssnet::Logger::setLogLevel(ssnet::Logger::LogLevel::DEBUG)" << std::endl;

    ssnet::Logger::setLogLevel(ssnet::Logger::LogLevel::DEBUG);

    LOG(DEBUG) << "Debug message";
    LOG(INFO) << "Info message";
    LOG(WARN) << "Warning message";
    LOG(ERROR) << "Error message";
    LOG(FATAL) << "Fatal message";

}

} // namespace

int main() {
    testLogLevelSetting();

    LOG(INFO) << "All tests passed!";
    return 0;
}
