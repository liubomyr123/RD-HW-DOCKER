#pragma once

#define ENABLE_LOG	0
#define ENABLE_LOG_DEBUG  0

#include <fstream>
#include <string>
// #include <sstream>

class Logger {
private:
    std::ofstream file;
    Logger();

public:
    static Logger& instance();
    void log(const std::string& type, const std::string& msg);
};


#if ENABLE_LOG

#define LOG_INFO(x) \
do { \
    std::ostringstream oss; \
    oss << x; \
    Logger::instance().log("", oss.str()); \
} while(0)

#define LOG_PROCESS(x) \
do { \
    std::ostringstream oss; \
    oss << x; \
    Logger::instance().log("🔄", oss.str()); \
} while(0)

#define LOG_ERROR(x) \
do { \
    std::ostringstream oss; \
    oss << x; \
    Logger::instance().log("❌", oss.str()); \
} while(0)

#define LOG_WARN(x) \
do { \
    std::ostringstream oss; \
    oss << x; \
    Logger::instance().log("⚠️", oss.str()); \
} while(0)

#define LOG_SUCCESS(x) \
do { \
    std::ostringstream oss; \
    oss << x; \
    Logger::instance().log("✅", oss.str()); \
} while(0)

#else

#define LOG_INFO(x)
#define LOG_PROCESS(x)
#define LOG_ERROR(x)
#define LOG_WARN(x)
#define LOG_SUCCESS(x)

#endif


#if ENABLE_LOG_DEBUG

#define LOG_DEBUG(x) \
do { \
    std::ostringstream oss; \
    oss << x; \
    Logger::instance().log("🐞", oss.str()); \
} while(0)

#else

#define LOG_DEBUG(x)

#endif