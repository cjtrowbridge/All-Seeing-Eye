import requests
import time

IP = "http://allseeingeye.local"
try:
    print(f"Connecting to {IP}/api/status...")
    r = requests.get(f"{IP}/api/status", timeout=5)
    if r.status_code == 200:
        print("Success! Response:")
        print(r.json())
    else:
        print(f"Error {r.status_code}")
except Exception as e:
    print(f"Failed to connect: {e}")
