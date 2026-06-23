#include "base/Logger.hpp"
#include <iostream>
#include "Common.hpp"

Logger::Logger() 
{
    file.open(LOGGER_FILE_NAME, std::ios::out | std::ios::trunc);
}

Logger& Logger::instance() 
{
    static Logger logger;
    return logger;
}

void Logger::log(const std::string& type, const std::string& msg) 
{
    std::string line = type + " " + msg;

    std::cout << line << '\n';
    file << line << '\n';
}