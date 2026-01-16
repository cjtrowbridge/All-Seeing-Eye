#ifndef PEER_MANAGER_H
#define PEER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <vector>
#include <utility> // for std::pair
#include <ArduinoJson.h>
#include <deque>

struct Peer {
    String hostname;
    String description; // Friendly name
    String ip;
    String cluster;
    String status;
    String task; // New field
    bool online;
    unsigned long lastSeen;
    unsigned long lastProbe; // timestamp of last successful /api/status check
    
    // BLE Ranging Data
    std::vector<int> bleRssiHistory; // Last 5 RSSI values (-99 if missing)
    float bleDistance = -1.0f;       // Distance estimate (-1 if unknown)
};

struct IgnoredHost {
    String ip;
    unsigned long ignoredAt;
};

class PeerManager {
public:
    static PeerManager& instance();

    void begin();
    void loop();
    
    // Returns the list of discovered peers as a JSON Array
    String getPeersAsJson();
    void populatePeers(JsonArray& arr); // Helper for /api/status aggregation
    
    // Called when an unknown host requests data
    void trackIncomingRequest(String ip);

    // Manual Tool
    bool pingHost(String ip);
    
    // Updates status for a specific peer
    bool probePeer(String ip);

    // Update BLE statistics for all peers based on scan results
    // Input: List of {hostname, rssi} found in the scan
    void updateBleStats(const std::vector<std::pair<String, int>>& foundPeers);

private:
    PeerManager();
    
    std::vector<Peer> _peers;
    std::vector<IgnoredHost> _ignored;
    std::deque<String> _verificationQueue;
    
    unsigned long _lastScan = 0;
    
    // Subnet Scanner State
    bool _subnetScanActive = true; 
    int _currentSubnetIndex = 1;
    unsigned long _lastProbeCheck = 0; // For background polling

    void discover();
    void processVerificationQueue();
    void runSubnetScanStep();
    void maintainPeers(); // New periodic maintenance
    
    bool isPeered(String ip);
    bool isIgnored(String ip);
    // bool verifyPeer(String ip); // Replaced by probePeer
};

#endif
