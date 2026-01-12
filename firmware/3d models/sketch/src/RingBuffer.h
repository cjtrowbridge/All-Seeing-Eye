#line 1 "C:\\Users\\CJ\\Documents\\GitHub\\All-Seeing-Eye\\firmware\\AllSeeingEye\\src\\RingBuffer.h"
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <Arduino.h>

class RingBuffer {
public:
    static RingBuffer& instance();

    // Allocate buffer in PSRAM
    // Default 1MB for now, can be increased to 4MB+
    bool begin(size_t sizeBytes = 1024 * 1024); 

    // Write data to the buffer. 
    // If buffer is full, it overwrites the oldest data (Head pushes Tail).
    // Thread Safe.
    size_t write(const uint8_t* data, size_t len);

    // Read bytes from the buffer (Consumption).
    // Returns number of bytes read.
    // Thread Safe.
    size_t read(uint8_t* dest, size_t len);

    // Peek at data without advancing tail
    size_t peek(uint8_t* dest, size_t len, size_t offset=0);

    // Number of bytes currently held
    size_t available();
    
    // Total capacity
    size_t capacity();

    void clear();

private:
    RingBuffer();
    
    uint8_t* _buffer;
    size_t _size;
    size_t _head; // Write index
    size_t _tail; // Read index
    size_t _count; // Current bytes stored
    
    SemaphoreHandle_t _mutex;
};

#endif
