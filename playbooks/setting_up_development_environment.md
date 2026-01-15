# Playbook: Setting Up Development Environment

*Status: Stable*

## Objective
This playbook allows a new developer or agent to set up the local machine for full contribution to the All Seeing Eye project.

## Prerequisites
*   Windows 10/11 (Project scripts are PowerShell-optimized).

## Step-by-Step Instructions

1.  **Install Arduino Environment**
    *   **Arduino IDE 2.x**: Install for the GUI and Drivers.
    *   **Arduino CLI**: Download `arduino-cli.exe`.
        *   *Config*: Currently, the scripts look for `arduino-cli` in a specific path in `AppData`. You may need to edit `upload_ota.ps1` `$CliPath` if yours is different.
    *   **ESP32 Core**: Install `esp32` by Espressif Systems via Board Manager (Version 3.3.3 recommended).

2.  **Install Python Dependencies**
    *   **Command**: `pip install requests zeroconf`
    *   These are used for the desktop Client and Cluster auditing.

3.  **Install OpenSCAD (Optional)**
    *   Required only if editing 3D models.

4.  **Configure Known Hosts**
    *   **File**: `firmware/known_hosts.txt`.
    *   **Action**: Create this file if it doesn't exist. Add the hostnames (e.g., `allseeingeye-123456`) or IPs of your local nodes, one per line.

5.  **Test Deployment Script**
    *   **Command**: `cd firmware; .\upload_ota.ps1`
    *   **Expected**: Code compiles. If no nodes are online, "Offline" is expected, but "Compilation Failed" is a failure.

## Verification
*   `arduino-cli version` returns a version.
*   `python --version` returns 3.x.
*   `upload_ota.ps1` successfully compiles the firmware.
