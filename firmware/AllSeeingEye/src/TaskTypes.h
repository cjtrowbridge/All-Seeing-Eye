#ifndef TASKTYPES_H
#define TASKTYPES_H

#include <Arduino.h>

enum TaskType {
    TASK_CRITICAL,    // Startup/HW Check
    TASK_USER,        // API requested
    TASK_CLUSTER,     // Coordinated action
    TASK_BACKGROUND   // Idle/Scanning
};

struct TaskDefinition {
    String id;             // e.g., "ble-ranging/survey"
    String name;           // e.g., "Device Survey"
    String pluginName;     // e.g., "BleRanging"
    String description;    // e.g., "Lists all nearby BLE MACs and payloads."
    String endpoint;       // e.g., "/api/task/ble-ranging/survey"
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
