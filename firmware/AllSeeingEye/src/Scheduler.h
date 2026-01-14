#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "TaskTypes.h"
#include <deque>
#include <Arduino.h>

class Scheduler {
public:
    static Scheduler& instance();
    
    void begin();
    void loop(); // Called by Kernel::loop()

    // Add a task to the queue
    void enqueue(RadioTask task);
    
    // Immediate execution (preemption logic can be improved later)
    void preempt(RadioTask task);

    // Get current status for API
    RadioTask getCurrentTask();
    std::deque<RadioTask> getQueue(); // Copy for API

private:
    Scheduler();
    
    std::deque<RadioTask> _queue;
    RadioTask _current;
    
    bool _isIdle = true;
    
    void switchToTask(RadioTask t);
    void startIdle();
};

#endif
