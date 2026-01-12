#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\Logger.cpp"
#include "Logger.h"

Logger& Logger::instance() {
    static Logger _instance;
    return _instance;
}

Logger::Logger() {
    _mutex = xSemaphoreCreateMutex();
}

void Logger::info(const char* tag, const char* format, ...) {
    char loc_buf[256];
    va_list arg;
    va_start(arg, format);
    vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(arg);
    addLog("INFO", tag, loc_buf);
}

void Logger::warn(const char* tag, const char* format, ...) {
    char loc_buf[256];
    va_list arg;
    va_start(arg, format);
    vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(arg);
    addLog("WARN", tag, loc_buf);
}

void Logger::error(const char* tag, const char* format, ...) {
    char loc_buf[256];
    va_list arg;
    va_start(arg, format);
    vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(arg);
    addLog("ERROR", tag, loc_buf);
}

void Logger::addLog(String level, const char* tag, const char* message) {
    // Format: [Millis][Level][Tag] Message
    String timestamp = String(millis());
    String logEntry = "[" + timestamp + "][" + level + "][" + String(tag) + "] " + String(message);

    // 1. Write to Serial (Standard Output)
    Serial.println(logEntry);

    // 2. Write to Memory Buffer (Protected)
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        _logs.push_back(logEntry);
        _currentMemoryUsage += logEntry.length();

        // Enforce Limits (Count OR Memory)
        while (_logs.size() > _maxLogs || _currentMemoryUsage > _maxMemoryUsage) {
            if (!_logs.empty()) {
                _currentMemoryUsage -= _logs.front().length();
                _logs.pop_front();
            } else {
                // Should not happen, but safety break
                _currentMemoryUsage = 0; 
                break; 
            }
        }
        xSemaphoreGive(_mutex);
    }
}

std::deque<String> Logger::getLogs() {
    std::deque<String> copy;
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        copy = _logs; // Create a safe copy for the API to serialize
        xSemaphoreGive(_mutex);
    }
    return copy;
}
