import requests
import time
import sys

IP = "http://allseeingeye.local"

def get_logs():
    try:
        # Fetch status to confirm plugin
        r_stat = requests.get(f"{IP}/api/status", timeout=2)
        status = r_stat.json()
        print(f"\n[STATUS] Uptime: {status.get('uptime')}ms | Plugin: {status.get('plugin')}")
        
        # Fetch logs
        r_logs = requests.get(f"{IP}/api/logs", timeout=2)
        if r_logs.status_code == 200:
            logs = r_logs.text.splitlines()
            print("--- Recent Radio Logs ---")
            # Show last 5 lines containing "RSS"
            count = 0
            for line in reversed(logs):
                if "RadioTest" in line:
                    print(line)
                    count += 1
                if count >= 5: break
        else:
            print("[ERROR] Could not fetch logs")
            
    except Exception as e:
        print(f"[ERROR] {e}")

if __name__ == "__main__":
    print(f"Polling {IP} for Radio Data...")
    for i in range(5):
        get_logs()
        time.sleep(2)
