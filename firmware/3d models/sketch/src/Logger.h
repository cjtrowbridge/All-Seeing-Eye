#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\Logger.h"
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <deque>

class Logger {
public:
    static Logger& instance();

    void info(const char* tag, const char* format, ...);
    void warn(const char* tag, const char* format, ...);
    void error(const char* tag, const char* format, ...);
    
    // Retrieve logs for the API (returns a copy of the buffer)
    std::deque<String> getLogs();

private:
    Logger();
    
    void addLog(String level, const char* tag, const char* message);
    
    std::deque<String> _logs;
    const size_t _maxLogs = 200; // Limit by Count
    const size_t _maxMemoryUsage = 20 * 1024; // Limit by RAM (20KB)
    size_t _currentMemoryUsage = 0;

    SemaphoreHandle_t _mutex;
};

#endif
