#ifndef PEER_MANAGER_H
#define PEER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <vector>
#include <ArduinoJson.h>
#include <deque>

struct Peer {
    String hostname;
    String ip;
    String cluster;
    String status;
    bool online;
    unsigned long lastSeen;
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
    
    // Called when an unknown host requests data
    void trackIncomingRequest(String ip);

    // Manual Tool
    bool pingHost(String ip);

private:
    PeerManager();
    
    std::vector<Peer> _peers;
    std::vector<IgnoredHost> _ignored;
    std::deque<String> _verificationQueue;
    
    unsigned long _lastScan = 0;
    
    // Subnet Scanner State
    bool _subnetScanActive = true; 
    int _currentSubnetIndex = 1;
    
    void discover();
    void processVerificationQueue();
    void runSubnetScanStep();
    
    bool isPeered(String ip);
    bool isIgnored(String ip);
    bool verifyPeer(String ip);
};

#endif
