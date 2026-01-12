# Playbook: Using Python API Client Best Practices

*Status: Draft*

## Objective
To ensure that all Python scripts interacting with the cluster are robust, handle network flakes gracefully, and respect the API standards.

## Best Practices

1.  **Use the `EyeClient` Class**
    *   Do not use `requests.get()` directly in script logic.
    *   Use the wrapper class in `api/client.py` (once implemented) which handles:
        *   Base URL formatting.
        *   JSON parsing.
        *   Exception handling.

2.  **Handle mDNS Latency**
    *   Resolving `.local` domains can take 1-2 seconds.
    *   **Tip**: If performing a high-frequency loop, verify the IP once and cache it, rather than resolving `hostname.local` on every request.

3.  **Respect Timeouts**
    *   ESP32 is single-threaded (mostly). If it's busy, it might delay.
    *   Set `timeout=5` (seconds) on requests.
    *   Catch `requests.exceptions.ConnectTimeout`.

4.  **Data Typing**
    *   The API returns strings for most values to be safe.
    *   Always cast `int(rssi)` before math.

## Example Pattern

```python
from client import EyeClient

def poll_node(hostname):
    eye = EyeClient(hostname)
    try:
        status = eye.get_status()
        print(f"{hostname} is doing {status['task']}")
    except Exception as e:
        print(f"Failed to reach {hostname}: {e}")
```

## Verification
*   Script runs without crashing when a node is unplugged.
*   Script handles invalid JSON gracefully.
