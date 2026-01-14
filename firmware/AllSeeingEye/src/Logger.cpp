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
        // Main RingBuffer logic
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

        // Head Logs Logic (Preserve first N logs)
        if (_headLogs.size() < _maxHeadLogs) {
            _headLogs.push_back(logEntry);
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

std::deque<String> Logger::getHeadLogs() {
    std::deque<String> copy;
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        copy = _headLogs; 
        xSemaphoreGive(_mutex);
    }
    return copy;
}

void Logger::populateLogs(JsonArray& arr) {
    if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Return only last 50 logs for /api/status to save bandwidth
        size_t start = (_logs.size() > 50) ? _logs.size() - 50 : 0;
        auto it = _logs.begin();
        std::advance(it, start);
        
        for (; it != _logs.end(); ++it) {
            arr.add(*it);
        }
        xSemaphoreGive(_mutex);
    }
}
