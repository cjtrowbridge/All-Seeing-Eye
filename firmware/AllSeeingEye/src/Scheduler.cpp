#include "Scheduler.h"
#include "PluginManager.h"
#include "Logger.h"

Scheduler& Scheduler::instance() {
    static Scheduler _instance;
    return _instance;
}

Scheduler::Scheduler() {}

void Scheduler::begin() {
    Logger::instance().info("Scheduler", "Starting...");
    // Init with Idle
    startIdle();
}

void Scheduler::loop() {
    // Check if current task is expired
    if (!_isIdle) {
        if (_current.durationMs > 0) { // 0 = infinite
            if (millis() - _current.startTime > _current.durationMs) {
                Logger::instance().info("Scheduler", "Task %s expired. Cleaning up.", _current.taskName.c_str());
                startIdle(); // This will trigger next task pick-up in next pass or immediately
            }
        }
    }

    // If currently Idle (or just finished), check Queue
    if (_isIdle && !_queue.empty()) {
        RadioTask next = _queue.front();
        _queue.pop_front();
        switchToTask(next);
    }
}

void Scheduler::enqueue(RadioTask task) {
    // Basic validation
    if (task.id.length() == 0) task.id = String(millis()); // fallback ID
    
    Logger::instance().info("Scheduler", "Enqueued Task: %s (%s)", task.taskName.c_str(), task.pluginName.c_str());
    _queue.push_back(task);
}

void Scheduler::preempt(RadioTask task) {
    // TODO: Handle clean abort of current task if it's high priority
    // For now, push to front and reset idle?
    // Proper preemption requires interrupting the running plugin.
    // PluginManager::loadPlugin() does handle teardown.
    
    Logger::instance().warn("Scheduler", "Preempting with Task: %s", task.taskName.c_str());
    _queue.push_front(task);
    
    // Force switch immediately?
    // If we just push front and invalid current, next loop picks it up.
    // However, if current is infinite, loop() might not pick it up.
    // Force switch:
    switchToTask(task);
}

void Scheduler::switchToTask(RadioTask t) {
    _current = t;
    _current.startTime = millis();
    _current.isRunning = true;
    _isIdle = (t.type == TASK_BACKGROUND); // Technically Idle is a task too

    Logger::instance().info("Scheduler", "Switching to Task: %s", t.taskName.c_str());

    // Create Plugin Instance
    ASEPlugin* p = PluginManager::instance().createPlugin(t.pluginName);
    
    // Pass params? (Future: p->setParams(t.paramsJson))
    
    // Load into Manager
    PluginManager::instance().loadPlugin(p, true);
}

void Scheduler::startIdle() {
    // Don't restart idle if we are already idle
    if (_current.pluginName == "SystemIdle" && _isIdle) return;

    RadioTask idle;
    idle.id = "IDLE-" + String(millis());
    idle.type = TASK_BACKGROUND;
    idle.pluginName = "SystemIdle";
    idle.taskName = "System Idle";
    idle.durationMs = 0; // Infinite

    switchToTask(idle);
}

RadioTask Scheduler::getCurrentTask() {
    return _current;
}

std::deque<RadioTask> Scheduler::getQueue() {
    return _queue;
}
