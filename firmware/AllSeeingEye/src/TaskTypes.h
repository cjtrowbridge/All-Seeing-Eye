#ifndef TASKTYPES_H
#define TASKTYPES_H

#include <Arduino.h>

enum TaskType {
    TASK_CRITICAL,    // Startup/HW Check
    TASK_USER,        // API requested
    TASK_CLUSTER,     // Coordinated action
    TASK_BACKGROUND   // Idle/Scanning
};

struct RadioTask {
    String id;           // UUID
    TaskType type;
    String pluginName;   // Class name key
    String taskName;     // Human readable description
    String paramsJson;   // JSON parameters
    unsigned long durationMs; 
    unsigned long createdAt;

    // Runtime state
    bool isRunning = false;
    unsigned long startTime = 0;
};

#endif
