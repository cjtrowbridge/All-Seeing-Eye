#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <deque>
#include <ArduinoJson.h>

class Logger {
public:
    static Logger& instance();

    void info(const char* tag, const char* format, ...);
    void warn(const char* tag, const char* format, ...);
    void error(const char* tag, const char* format, ...);
    
    // Retrieve logs for the API (returns a copy of the buffer)
    std::deque<String> getLogs();
    std::deque<String> getHeadLogs();
    void populateLogs(JsonArray& arr); // Helper for /api/status aggregation

private:
    Logger();
    
    void addLog(String level, const char* tag, const char* message);
    
    std::deque<String> _logs;
    std::deque<String> _headLogs;
    const size_t _maxLogs = 200; // Limit by Count
    const size_t _maxHeadLogs = 50; // Keep the first 50 logs forever
    const size_t _maxMemoryUsage = 20 * 1024; // Limit by RAM (20KB)
    size_t _currentMemoryUsage = 0;

    SemaphoreHandle_t _mutex;
};

#endif
