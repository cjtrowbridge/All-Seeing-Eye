from __future__ import annotations

import argparse
import json
import sys
from typing import Any, Dict, Optional

from client import EyeClient
from discovery import discover_hosts
from manager import HostManager


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="All Seeing Eye CLI")
    parser.add_argument("--list", action="store_true", help="List known hosts")
    parser.add_argument("--host", help="Target host by IP or hostname (quote if spaces)")
    parser.add_argument("--json", action="store_true", help="Output raw JSON")
    parser.add_argument("--ble-ranging", action="store_true", help="Get latest BLE ranging scan")
    parser.add_argument("action", nargs="?", help="HTTP method (get/post)")
    parser.add_argument("endpoint", nargs="?", help="API endpoint, e.g. /api/status")
    parser.add_argument("payload", nargs="?", help="JSON payload for POST")
    return parser.parse_args()


def handle_list(manager: HostManager, output_json: bool) -> int:
    hosts = manager.list_hosts()
    if output_json:
        print(json.dumps(hosts, indent=2))
        return 0

    for index, host in enumerate(hosts):
        default_marker = " (Default)" if index == 0 else ""
        print(
            f"{host['ip_address']} | {host['hostname']} | {host['description']}{default_marker}"
        )
    return 0


def handle_request(
    client: EyeClient,
    action: str,
    endpoint: str,
    payload: Optional[str],
    output_json: bool,
) -> int:
    data: Optional[Dict[str, Any]] = None
    if payload:
        try:
            data = json.loads(payload)
        except json.JSONDecodeError as exc:
            print(f"Invalid JSON payload: {exc}", file=sys.stderr)
            return 1

    response = client.request(action, endpoint, payload=data)
    if output_json:
        print(json.dumps(response, indent=2))
    else:
        print(json.dumps(response, indent=2))

    if not response.get("ok", False):
        return 1
    return 0


def ensure_hosts(manager: HostManager) -> None:
    if manager.list_hosts():
        return
    discover_hosts(manager)


def main() -> int:
    args = parse_args()
    manager = HostManager()

    if args.list:
        ensure_hosts(manager)
        return handle_list(manager, args.json)

    ensure_hosts(manager)
    try:
        host = manager.resolve_host(args.host)
    except Exception as exc:
        print(str(exc), file=sys.stderr)
        return 1

    client = EyeClient(host["ip_address"])

    if args.ble_ranging:
        response = client.get_ble_ranging()
        print(json.dumps(response, indent=2))
        return 0 if response.get("ok", False) else 1

    if not args.action or not args.endpoint:
        print("Missing action or endpoint. Use --list or provide command.", file=sys.stderr)
        return 1

    return handle_request(client, args.action, args.endpoint, args.payload, args.json)


if __name__ == "__main__":
    sys.exit(main())
