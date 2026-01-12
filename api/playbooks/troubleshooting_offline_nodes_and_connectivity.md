# Playbook: Troubleshooting Offline Nodes and Connectivity

*Status: Stable*

## Objective
This playbook provides a systematic checklist for **Agents** to diagnose "Offline" or "Unknown" nodes. Since Agents cannot see the physical devices, this workflow combines **direct terminal actions** with **questions for the User**.

## Prerequisites
*   Powershell terminal access.
*   User presence (for physical inspection).

## Step-by-Step Triage

1.  **Phase 1: Agent Active Reconnaissance (Do this first)**
    *   **Goal**: Determine if the node is network-reachable without bothering the user yet.
    *   **Action 1**: Check if the host resolves.
        *   Command: `ping -n 1 [hostname].local`
    *   **Action 2**: Scan IP Table if hostname fails.
        *   Command: `arp -a`
        *   *Analysis*: Search output for Espressif MAC OUI (`48:E7`, `54:32`, etc).
    *   **Result**: 
        *   If responding to Ping: Node is Alive (Network Layer OK). Problem is likely Application Layer (Web Server crashed).
        *   If NO response: Node is network-dead. Proceed to Phase 2.

2.  **Phase 2: User Physical Inspection (Ask the User)**
    *   **Goal**: Determine if the node has power or has crashed hard.
    *   **Agent Prompt**: "I cannot reach node `[hostname]`. Please look at the device and tell me:"
        1.  "Is the **Red Power LED** on?" (Power Good)
        2.  "Is the **Blue Status LED** blinking?" (Heartbeat = Kernel Running).
        3.  "Is the **Blue Status LED** solid on or solid off?" (Kernel Hung/Panic).

3.  **Phase 3: Serial Debugging (Collaborative)**
    *   **Goal**: Capture boot logs to find bootloops or WiFi errors.
    *   **Agent Action**: Ask the user to connect the device via USB.
    *   **Agent Action**: Monitor the Serial Port (if capable) or ask user to provide logs.
        *   *Instruction*: "Please check the Serial Monitor at 115200 baud. Does it hang at 'Connecting to WiFi...'?"

## Common Resolutions & Fixes

*   **Symptom**: Node Stuck in "Unknown" in Peer List.
    *   *Cause*: mDNS announced existence, but HTTP Status Poll failed.
    *   *Agent Action*: Run a manual curl to check the endpoint: `curl -v http://[hostname].local/api/status`.

*   **Symptom**: Deployment Script says "OFFLINE".
    *   *Cause*: Windows PowerShell often fails to resolve `.local` domains during `Test-Connection` even if they work in the browser.
    *   *Agent Action*: Suggest replacing the hostname in `known_hosts.txt` with the raw IP address found in `arp -a`.

## Verification
*   User confirms LEDs are normal.
*   Agent successfully pings the node.
*   Agent successfully retrieves JSON from `/api/status`.
