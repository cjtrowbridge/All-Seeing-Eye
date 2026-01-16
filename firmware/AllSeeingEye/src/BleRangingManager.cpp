#include "BleRangingManager.h"
#include "PeerManager.h"
#include "Logger.h"
#include "Config.h"
#include <time.h>
#include "esp_mac.h"

// Callback class for BLE scanning
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        String address = advertisedDevice.getAddress().toString().c_str();
        int rssi = advertisedDevice.getRSSI();
        String serviceUUID = "";
        String name = "";
        
        if (advertisedDevice.haveServiceUUID()) {
            serviceUUID = advertisedDevice.getServiceUUID().toString().c_str();
        }
        if (advertisedDevice.haveName()) {
            name = advertisedDevice.getName().c_str();
        }
        
        std::string mfgData = "";
        if (advertisedDevice.haveManufacturerData()) {
            mfgData = advertisedDevice.getManufacturerData().c_str();
        }

        int txPower = 0;
        if (advertisedDevice.haveTXPower()) {
            txPower = advertisedDevice.getTXPower();
        }
        
        BleRangingManager::instance().handleDeviceFound(address, rssi, txPower, serviceUUID, name, mfgData);
    }
};

BleRangingManager& BleRangingManager::instance() {
    static BleRangingManager instance;
    return instance;
}

void BleRangingManager::begin() {
    _peerCount = 0;
    _bssidCount = 0;
    _lastScanEpoch = 0;
    _lastScanMs = millis();
    
    // Initialize BLE with specific hostname
    String hostname = Config::instance().getHostname();
    if (hostname.length() == 0) hostname = "AllSeeingEye";
    BLEDevice::init(hostname.c_str());

    BLEDevice::setPower(ESP_PWR_LVL_P9); // Max power
    
    _pBLEScan = BLEDevice::getScan();
    _pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    _pBLEScan->setActiveScan(true); // Active scan uses more power, gets name/more data
    _pBLEScan->setInterval(100);
    _pBLEScan->setWindow(99);  // Must be <= Interval

    // Setup Advertising
    _pBLEAdvertising = BLEDevice::getAdvertising();
    // We add our Service UUID so peers can identify us
    _pBLEAdvertising->addServiceUUID(ASE_SERVICE_UUID);
    _pBLEAdvertising->setScanResponse(true);
    _pBLEAdvertising->setMinPreferred(0x06);  // iPhone connection helper
    _pBLEAdvertising->setMinPreferred(0x12);
    
    BLEDevice::startAdvertising();

    Logger::instance().info("BleRanging", "Manager initialized (BLE Active)");
}

void BleRangingManager::stop() {
    BLEDevice::deinit(true); // Release memory
    _pBLEScan = nullptr;
    _pBLEAdvertising = nullptr;
    Logger::instance().info("BleRanging", "BLE De-initialized (Memory Released)");
}

void BleRangingManager::loop() {
    if (!_config.enabled) {
        return;
    }

    time_t epoch = time(nullptr);
    if (epoch <= 0) {
        return;
    }

    // Synchronized Scanning Window (UTC % 10 == 0)
    if ((epoch % 10) == 0 && _lastScanEpoch != static_cast<uint32_t>(epoch)) {
        _lastScanEpoch = static_cast<uint32_t>(epoch);
        
        Logger::instance().info("BleRanging", "Starting Scan (Blocking 1s)");
        
        // Blocking scan for 1 second (standard BLEScan limitation taking seconds)
        // If we want sub-second, we'd need a FreeRTOS task or manual start/stop
        // For atomic implementation, we accept the 1s block for now.
        BLEScanResults* foundDevices = _pBLEScan->start(1, false);
        
        // Update PeerManager with scan results
        if (foundDevices) {
            std::vector<std::pair<String, int>> foundPeers;
            int count = foundDevices->getCount();
            for (int i = 0; i < count; i++) {
                BLEAdvertisedDevice dev = foundDevices->getDevice(i);
                if (dev.haveName()) {
                    foundPeers.push_back(std::make_pair(String(dev.getName().c_str()), dev.getRSSI()));
                }
            }
            PeerManager::instance().updateBleStats(foundPeers);
            
            Logger::instance().info("BleRanging", "Scan Complete. Found %d devices", count);
        } else {
            Logger::instance().info("BleRanging", "Scan Complete. Found 0 devices");
        }
        _pBLEScan->clearResults();   // Release memory
    }
}

BleRangingConfig BleRangingManager::getConfig() const {
    return _config;
}

void BleRangingManager::setConfig(const BleRangingConfig& config) {
    _config = config;
    Logger::instance().info("BleRanging", "Config updated enabled=%d interval=%lu window=%lu adv=%lu",
        _config.enabled,
        _config.scanIntervalMs,
        _config.scanWindowMs,
        _config.advertiseIntervalMs);
}

void BleRangingManager::updatePeer(const BleRangingPeer& peer) {
    for (uint8_t i = 0; i < _peerCount; ++i) {
        if (_peers[i].peerId == peer.peerId) {
            _peers[i] = peer;
            return;
        }
    }

    if (_peerCount < kMaxPeers) {
        _peers[_peerCount] = peer;
        _peerCount++;
    } else {
        for (uint8_t i = 1; i < kMaxPeers; ++i) {
            _peers[i - 1] = _peers[i];
        }
        _peers[kMaxPeers - 1] = peer;
    }
}

void BleRangingManager::clearPeers() {
    _peerCount = 0;
}

void BleRangingManager::updateBssid(const BleRangingBssid& bssid) {
    for (uint8_t i = 0; i < _bssidCount; ++i) {
        if (_bssids[i].bssid == bssid.bssid) {
            _bssids[i] = bssid;
            return;
        }
    }

    if (_bssidCount < kMaxBssids) {
        _bssids[_bssidCount] = bssid;
        _bssidCount++;
    } else {
        for (uint8_t i = 1; i < kMaxBssids; ++i) {
            _bssids[i - 1] = _bssids[i];
        }
        _bssids[kMaxBssids - 1] = bssid;
    }
}

void BleRangingManager::clearBssids() {
    _bssidCount = 0;
}

void BleRangingManager::populateStatus(JsonObject& obj) const {
    obj["enabled"] = _config.enabled;
    obj["scan_interval_ms"] = _config.scanIntervalMs;
    obj["scan_window_ms"] = _config.scanWindowMs;
    obj["advertise_interval_ms"] = _config.advertiseIntervalMs;
    obj["last_scan"] = _lastScanEpoch;
    obj["service_uuid"] = ASE_SERVICE_UUID;
    
    // Get Local MAC
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    obj["local_bssid"] = String(macStr);

    JsonArray peers = obj.createNestedArray("peers");
    for (uint8_t i = 0; i < _peerCount; ++i) {
        JsonObject p = peers.createNestedObject();
        p["peer_id"] = _peers[i].peerId;
        p["rssi"] = _peers[i].rssi;
        p["distance_m"] = _peers[i].distanceM;
        p["seen_at"] = _peers[i].seenAt;
        p["name"] = _peers[i].name;
        p["service_uuid"] = _peers[i].serviceUuid;
    }

    JsonArray bssids = obj.createNestedArray("bssids");
    for (uint8_t i = 0; i < _bssidCount; ++i) {
        JsonObject b = bssids.createNestedObject();
        b["bssid"] = _bssids[i].bssid;
        b["rssi"] = _bssids[i].rssi;
        b["tx_power"] = _bssids[i].txPower;
        b["seen_at"] = _bssids[i].seenAt;
    }
}

void BleRangingManager::handleDeviceFound(const String& address, int rssi, int txPower, const String& serviceUUID, const String& name, const std::string& manufacturerData) {
    // 1. Always add to raw BSSID list
    BleRangingBssid bssid;
    bssid.bssid = address;
    bssid.rssi = rssi;
    bssid.txPower = txPower;
    bssid.seenAt = millis();
    
    updateBssid(bssid);

    // 2. If it matches our Service UUID, it is a Peer
    if (serviceUUID.equalsIgnoreCase(ASE_SERVICE_UUID)) {
        BleRangingPeer peer;
        peer.peerId = address; // Use MAC as ID for now
        peer.rssi = rssi;
        peer.distanceM = pow(10.0, ((-69.0 - rssi) / (20.0))); // Basic estimation: 10^((TxPower - RSSI) / (10 * n))
        peer.seenAt = millis();
        peer.name = name;
        peer.serviceUuid = serviceUUID;
        updatePeer(peer);
    }
}
