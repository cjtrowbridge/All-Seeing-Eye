#ifndef BLERANGINGMANAGER_H
#define BLERANGINGMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertising.h>

// Generated UUID for All Seeing Eye Service
#define ASE_SERVICE_UUID "180f6f62-2b31-4307-b353-9d115e5c707d"

struct BleRangingConfig {
    bool enabled = true;
    uint32_t scanIntervalMs = 10000;
    uint32_t scanWindowMs = 200;      // NOTE: Standard BLEScan uses seconds. We may need to manage manual start/stop for sub-second precision
    uint32_t advertiseIntervalMs = 1000;
};

struct BleRangingPeer {
    String peerId;
    int rssi = 0;
    float distanceM = 0.0f;
    uint32_t seenAt = 0;
    String name;
    String serviceUuid;
};

struct BleRangingBssid {
    String bssid;
    int rssi = 0;
    int txPower = 0;
    uint32_t seenAt = 0;
};

class BleRangingManager {
public:
    static BleRangingManager& instance();

    void begin();
    void stop();
    void loop();

    BleRangingConfig getConfig() const;
    void setConfig(const BleRangingConfig& config);

    void updatePeer(const BleRangingPeer& peer);
    void clearPeers();

    void updateBssid(const BleRangingBssid& bssid);
    void clearBssids();

    void populateStatus(JsonObject& obj) const;

    // Helper for the callback to insert data
    void handleDeviceFound(const String& address, int rssi, int txPower, const String& serviceUUID, const String& name, const std::string& manufacturerData);

private:
    BleRangingManager() = default;

    static const uint8_t kMaxPeers = 8;
    static const uint8_t kMaxBssids = 16;

    BleRangingConfig _config;
    BleRangingPeer _peers[kMaxPeers];
    uint8_t _peerCount = 0;
    BleRangingBssid _bssids[kMaxBssids];
    uint8_t _bssidCount = 0;
    uint32_t _lastScanEpoch = 0;
    uint32_t _lastScanMs = 0;
    bool _isScanning = false;
    uint32_t _scanStartTime = 0;
    
    BLEScan* _pBLEScan = nullptr;
    BLEAdvertising* _pBLEAdvertising = nullptr;
};

#endif
