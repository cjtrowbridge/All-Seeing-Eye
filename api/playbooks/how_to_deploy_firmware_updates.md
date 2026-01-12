# Playbook: How to Deploy Firmware Updates

*Status: Stable*

## Objective
This playbook describes the process for compiling, packing web assets, and deploying firmware updates to the entire All Seeing Eye fleet using the `upload_ota.ps1` script.

## Prerequisites
*   **Operating System**: Windows (PowerShell 5.1+).
*   **Tools**: Arduino CLI, Python 3 (for packing assets), `espota.exe`.
*   **Network**: Desktop must be on the same local network as the nodes.
*   **Context**: You must be in the `firmware/` directory of the repository.

## Step-by-Step Instructions

1.  **Navigate to Firmware Directory**
    *   Command: `cd firmware`
    *   Reason: The script uses `$PSScriptRoot` relative paths, but running it from the root without `cd` can sometimes cause context issues.

2.  **Execute the Deployment Script**
    *   Command: `.\upload_ota.ps1`
    *   *Note*: If you run into Execution Policy errors, use: `PowerShell -ExecutionPolicy Bypass -File .\upload_ota.ps1`

3.  **Monitor the Output**
    *   **Phase 1: Packing Web Assets**: The script runs `pack_web.py`. Ensure it says "[PACK] Done."
    *   **Phase 2: Compiling Firmware**: The script calls `arduino-cli`. This process takes 30-60 seconds.
        *   *If it fails*: The error log is at `build/compile.log`. Read it to diagnose (usually missing libraries or syntax errors).
    *   **Phase 3: Deployment**: The script iterates through `known_hosts.txt`.

4.  **Verify Success**
    *   **Success**: Output shows `Target: [hostname] ... Status: ONLINE ... SUCCESS`.
    *   **Failure**: Output shows `FAILED`. Check `build/upload_[hostname].log` for details.

## Troubleshooting

*   **"Index.html not found"**: You are likely running the script from the repo root. `cd firmware` first.
*   **"Compilation Failed"**: Open `build/compile.log`.
    *   *Symptom*: `fatal error: header.h: No such file`.
    *   *Fix*: Check `libraries/` folder or `arduino-cli` library include paths.
*   **"Offline (Skipping)"**:
    *   Check if the node is powered on.
    *   Check if the hostname is resolvable (`ping [hostname]`).
    *   If mDNS is broken, try using the IP address in `known_hosts.txt` instead of the hostname.

## Verification
1.  Visit `http://[hostname].local` (or IP) in a browser.
2.  Check the "API Status" at `http://[hostname].local/api/status` to confirm the version or build time matches your update.
