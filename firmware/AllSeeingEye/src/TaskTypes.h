#ifndef TASKTYPES_H
#define TASKTYPES_H

#include <Arduino.h>
#include <vector>

enum TaskInputValueType {
    INPUT_VALUE_NONE,
    INPUT_VALUE_NUMBER,
    INPUT_VALUE_BOOL,
    INPUT_VALUE_TEXT
};

struct TaskInputOption {
    String label;
    String value;
};

struct TaskInputDefinition {
    String name;
    String label;
    String type; // number, select, boolean, text
    bool required = false;

    TaskInputValueType defaultType = INPUT_VALUE_NONE;
    float defaultNumber = 0.0f;
    bool defaultBool = false;
    String defaultText = "";

    bool hasStep = false;
    float step = 0.0f;
    bool hasMin = false;
    float min = 0.0f;
    bool hasMax = false;
    float max = 0.0f;

    std::vector<TaskInputOption> options;
};

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
    std::vector<TaskInputDefinition> inputs;
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
