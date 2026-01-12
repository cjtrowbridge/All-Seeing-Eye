#include "PeerManager.h"
#include "Logger.h"
#include "Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

PeerManager& PeerManager::instance() {
    static PeerManager _instance;
    return _instance;
}

PeerManager::PeerManager() {}

void PeerManager::begin() {
    Logger::instance().info("Peers", "Peer Discovery Started (mDNS)");
    // MDNS.begin is handled in Kernel, so we just assume it's ready or will be.
}

void PeerManager::loop() {
    // 1. Process Verification Queue (High Priority)
    if (!_verificationQueue.empty()) {
        processVerificationQueue();
    } 
    // 2. Run Subnet Scan (Lower Priority, only if no peers found yet)
    else if (_peers.empty() && _subnetScanActive) {
        runSubnetScanStep();
    }
    
    // 3. Maintenance (Probe known peers for status updates)
    if (millis() - _lastProbeCheck > 2000) { // Check one peer every 2 seconds
        _lastProbeCheck = millis();
        maintainPeers();
    }

    // Scan every 30 seconds
    if (millis() - _lastScan > 30000) {
        _lastScan = millis();
        discover();
    }
}

void PeerManager::maintainPeers() {
    if (_peers.empty()) return;

    // Find the peer that needs probing the most
    // Criteria: "Unknown" status OR oldest lastProbe
    int targetIdx = -1;
    unsigned long oldestTime = millis();
    
    for (int i = 0; i < _peers.size(); i++) {
        if (!_peers[i].online) continue; // Don't spam offline peers? Or maybe do to see if they are back?
        // Let's stick to online ones for status updates first.
        
        // Priority: Status is Unknown
        if (_peers[i].status == "Unknown") {
            targetIdx = i;
            break; // Found high priority
        }
        
        // Otherwise: Oldest probe
        if (_peers[i].lastProbe < oldestTime) {
            oldestTime = _peers[i].lastProbe;
            targetIdx = i;
        }
    }
    
    // If we found a candidate, and it hasn't been probed effectively recently (e.g. 10 seconds)
    // Actually, if we use a simple round-robin via timestamps, just probe the chosen one.
    if (targetIdx >= 0) {
        // Debounce: If probing happened very recently, skip?
        // But we enter here every 2s. If we have 10 peers, update rate is 20s. That's fine.
        Logger::instance().info("Peers", "Probing %s for status...", _peers[targetIdx].ip.c_str());
        probePeer(_peers[targetIdx].ip);
    }
}

void PeerManager::trackIncomingRequest(String ip) {
    if (isPeered(ip) || isIgnored(ip)) return;
    
    // Add to queue for verification
    // Check if already in queue to avoid duplicates
    for(const auto& qIp : _verificationQueue) {
        if(qIp == ip) return;
    }
    _verificationQueue.push_back(ip);
}

void PeerManager::processVerificationQueue() {
    if(_verificationQueue.empty()) return;

    String targetIp = _verificationQueue.front();
    _verificationQueue.pop_front();

    Logger::instance().info("Peers", "Verifying potential peer: %s", targetIp.c_str());
    if (probePeer(targetIp)) {
        // Peer added inside probePeer if successful
        Logger::instance().info("Peers", "Verified! Added %s", targetIp.c_str());
    } else {
        // Add to ignore list
        IgnoredHost ign;
        ign.ip = targetIp;
        ign.ignoredAt = millis();
        _ignored.push_back(ign);
        Logger::instance().info("Peers", "Not a peer. Ignoring %s", targetIp.c_str());
    }
}

void PeerManager::runSubnetScanStep() {
    if (!WiFi.isConnected()) return;

    // Get Local Subnet
    IPAddress local = WiFi.localIP();
    IPAddress mask = WiFi.subnetMask();
    
    // Simple logic for /24 standard networks
    // Construct target IP
    IPAddress target = local;
    target[3] = _currentSubnetIndex;

    // Skip self and gateway (usually .1)
    if (target != local && target != WiFi.gatewayIP()) {
        String targetStr = target.toString();
        
        // Skip if ignored
        if (!isIgnored(targetStr)) {
            // Logger::instance().info("Peers", "Scanning Subnet: %s", targetStr.c_str()); // Verbose
            if (probePeer(targetStr)) {
                Logger::instance().info("Peers", "Subnet Scan found peer! %s", targetStr.c_str());
                // We found one! Current requirement says "until it finds one". 
                // Implicitly, if we find one, the _peers list is not empty, so the loop condition `_peers.empty()` stops this scan.
            } else {
                 // Add to ignore list to prevent re-scanning in this session if we loop or restart scan
                 // For now, let's not fill the ignore list with the whole subnet, just iterate.
            }
        }
    }

    _currentSubnetIndex++;
    if (_currentSubnetIndex > 254) {
        _currentSubnetIndex = 1; // Restart or stop? 
        // "until it finds one". If we scanned whole subnet and found none, maybe pause or restart?
        // Let's pause for a while to save bandwidth. But the loop handles 30s intervals for mDNS.
        // Let's loop but slower? Or just let it loop.
        // For compliance with "seek ... until it finds one", we loop.
    }
}

bool PeerManager::probePeer(String ip) {
    HTTPClient http;
    // Short timeout to not block too long
    http.setTimeout(2000); 
    
    String url = "http://" + ip + "/api/status";
    http.begin(url);
    
    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error && doc.containsKey("hostname")) {
             // Valid Peer!
             String pHostname = doc["hostname"].as<String>();
             String pCluster = doc["clusterName"] | "Default"; 
             String pStatus = doc["status"] | "Unknown";
             String pTask = doc["task"] | "Unknown Task";

             // Check if exists
             bool found = false;
             for(auto& peer : _peers) {
                 if(peer.ip == ip) {
                     peer.hostname = pHostname;
                     peer.cluster = pCluster;
                     peer.status = pStatus;
                     peer.task = pTask;
                     peer.online = true;
                     peer.lastSeen = millis();
                     peer.lastProbe = millis();
                     found = true;
                     break;
                 }
             }

             if(!found) {
                 Peer p;
                 p.hostname = pHostname;
                 p.ip = ip;
                 p.cluster = pCluster;
                 p.status = pStatus;
                 p.task = pTask;
                 p.online = true;
                 p.lastSeen = millis();
                 p.lastProbe = millis();
                 _peers.push_back(p);
             }
             
             http.end();
             return true;
        }
    }
    
    http.end();
    return false;
}

bool PeerManager::isPeered(String ip) {
    for (const auto& p : _peers) {
        if (p.ip == ip) return true;
    }
    return false;
}

bool PeerManager::isIgnored(String ip) {
    long timeout = Config::instance().getInt("peer_ignore_hours", 12) * 3600 * 1000;
    
    for (auto it = _ignored.begin(); it != _ignored.end(); ) {
        if (it->ip == ip) {
            if (millis() - it->ignoredAt < timeout) {
                return true;
            } else {
                // Expired
                it = _ignored.erase(it);
                return false;
            }
        } else {
            ++it;
        }
    }
    return false;
}

bool PeerManager::pingHost(String ip) {
    // Utility for the new API
    // Using HTTP check as "Ping" for now since raw ICMP might fail on permissions
    HTTPClient http;
    http.setTimeout(1000);
    http.begin("http://" + ip + "/api/status");
    int code = http.GET();
    http.end();
    return (code == 200);
}

void PeerManager::discover() {
    // Logger::instance().info("Peers", "Scanning for _allseeingeye._tcp...");
    
    int n = MDNS.queryService("allseeingeye", "tcp");
    if (n == 0) {
        // No services found
        return;
    }

    // Process results
    for (int i = 0; i < n; ++i) {
        String hostname = MDNS.hostname(i);
        String ip = MDNS.address(i).toString();
        
        // Extract Cluster from TXT
        String peerCluster = "Default";
        if (MDNS.hasTxt(i, "cluster")) {
             peerCluster = MDNS.txt(i, "cluster");
        }

        // Check if we already know this peer
        bool known = false;
        for (auto &p : _peers) {
            if (p.ip == ip) {
                p.hostname = hostname;
                p.cluster = peerCluster; // Update cluster
                p.lastSeen = millis();
                p.online = true;
                known = true;
                break;
            }
        }

        // Add new peer
        if (!known) {
            Logger::instance().info("Peers", "New Peer Discovered: %s (%s) Cluster: %s", hostname.c_str(), ip.c_str(), peerCluster.c_str());
            Peer p;
            p.hostname = hostname;
            p.ip = ip;
            p.cluster = peerCluster;
            p.status = "Unknown";
            p.online = true;
            p.lastSeen = millis();
            _peers.push_back(p);
        }
    }
}

String PeerManager::getPeersAsJson() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (const auto &p : _peers) {
        JsonObject obj = arr.add<JsonObject>();
        obj["hostname"] = p.hostname;
        obj["ip"] = p.ip;
        obj["cluster"] = p.cluster;
        obj["status"] = p.status;
        obj["task"] = p.task;
        obj["lastProbe"] = p.lastProbe;
        
        // Mark as offline if not seen in 2 minutes (scans happen every 30s)
        bool isOnline = (millis() - p.lastSeen < 120000);
        obj["online"] = isOnline;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}
