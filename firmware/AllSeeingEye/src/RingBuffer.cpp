#include "RingBuffer.h"
#include "Logger.h"

RingBuffer& RingBuffer::instance() {
    static RingBuffer _instance;
    return _instance;
}

RingBuffer::RingBuffer() : _buffer(nullptr), _size(0), _head(0), _tail(0), _count(0) {
    _mutex = xSemaphoreCreateMutex();
}

bool RingBuffer::begin(size_t sizeBytes) {
    if (_buffer != nullptr) {
        Logger::instance().warn("RingBuffer", "Already initialized");
        return true; 
    }

    Logger::instance().info("RingBuffer", "Allocating %d bytes in PSRAM...", sizeBytes);
    
    // Allocate in PSRAM (Capabilities: MALLOC_CAP_SPIRAM)
    _buffer = (uint8_t*) heap_caps_malloc(sizeBytes, MALLOC_CAP_SPIRAM);
    
    if (_buffer == nullptr) {
        Logger::instance().error("RingBuffer", "PSRAM Allocation FAILED! Falling back to Heap (small)...");
        // Fallback to small internal RAM buffer just so we don't crash
        sizeBytes = 16 * 1024; 
        _buffer = (uint8_t*) malloc(sizeBytes);
        if (_buffer == nullptr) {
            Logger::instance().error("RingBuffer", "CRITICAL: RAM Allocation FAILED");
            return false;
        }
    }

    _size = sizeBytes;
    _head = 0;
    _tail = 0;
    _count = 0;
    
    Logger::instance().info("RingBuffer", "Initialized. Capacity: %d bytes", _size);
    return true;
}

size_t RingBuffer::write(const uint8_t* data, size_t len) {
    if (!_buffer) return 0;
    
    xSemaphoreTake(_mutex, portMAX_DELAY);

    // If data is larger than entire buffer, just take the last chunk
    if (len > _size) {
        data += (len - _size);
        len = _size;
    }

    for (size_t i = 0; i < len; i++) {
        _buffer[_head] = data[i];
        
        _head = (_head + 1) % _size;
        
        if (_count < _size) {
            _count++;
        } else {
            // Buffer full, we overwrote the tail
            _tail = (_tail + 1) % _size;
        }
    }

    xSemaphoreGive(_mutex);
    return len;
}

size_t RingBuffer::read(uint8_t* dest, size_t len) {
    if (!_buffer || _count == 0) return 0;

    xSemaphoreTake(_mutex, portMAX_DELAY);

    size_t actualRead = (len > _count) ? _count : len;
    
    for (size_t i = 0; i < actualRead; i++) {
        dest[i] = _buffer[_tail];
        _tail = (_tail + 1) % _size;
    }
    
    _count -= actualRead;
    
    xSemaphoreGive(_mutex);
    return actualRead;
}

size_t RingBuffer::peek(uint8_t* dest, size_t len, size_t offset) {
    if (!_buffer || _count == 0) return 0;

    xSemaphoreTake(_mutex, portMAX_DELAY);
    
    if (offset >= _count) {
        xSemaphoreGive(_mutex);
        return 0;
    }

    size_t actualRead = (len > (_count - offset)) ? (_count - offset) : len;
    size_t currentPeekIndex = (_tail + offset) % _size;

    for (size_t i = 0; i < actualRead; i++) {
        dest[i] = _buffer[currentPeekIndex];
        currentPeekIndex = (currentPeekIndex + 1) % _size;
    }

    xSemaphoreGive(_mutex);
    return actualRead;
}

size_t RingBuffer::available() {
    // Atomic read of byte
    return _count; 
    // Strictly should take mutex but size_t read is usually atomic enough for status check
}

size_t RingBuffer::capacity() {
    return _size;
}

void RingBuffer::clear() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _head = 0;
    _tail = 0;
    _count = 0;
    xSemaphoreGive(_mutex);
}
