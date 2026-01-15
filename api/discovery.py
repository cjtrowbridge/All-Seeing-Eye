from __future__ import annotations

import json
import socket
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Dict, List, Optional, Set, Tuple

import requests

from manager import HostManager

DEFAULT_TIMEOUT_SECONDS = 2.0


def discover_hosts(
    manager: HostManager,
    use_mdns: bool = True,
    use_scan: bool = True,
    timeout_seconds: float = DEFAULT_TIMEOUT_SECONDS,
) -> List[Dict[str, str]]:
    discovered: List[Dict[str, str]] = []

    if use_mdns:
        for ip, hostname in mdns_discover(timeout_seconds=timeout_seconds):
            record = identify_host(ip, timeout_seconds=timeout_seconds)
            manager.add_host(
                ip,
                record.get("hostname", hostname or ip),
                record.get("description", ""),
            )
            discovered.append(manager.resolve_host(ip))

    if use_scan:
        subnet_ips = scan_subnet(timeout_seconds=timeout_seconds)
        for ip in subnet_ips:
            record = identify_host(ip, timeout_seconds=timeout_seconds)
            manager.add_host(
                ip,
                record.get("hostname", ip),
                record.get("description", ""),
            )
            discovered.append(manager.resolve_host(ip))

    for host in manager.list_hosts():
        crawl_peers(manager, host["ip_address"], timeout_seconds=timeout_seconds)

    return discovered


def mdns_discover(timeout_seconds: float = DEFAULT_TIMEOUT_SECONDS) -> Set[Tuple[str, str]]:
    try:
        from zeroconf import ServiceBrowser, ServiceStateChange, Zeroconf
    except Exception:
        return set()

    zeroconf = Zeroconf()
    results: Set[Tuple[str, str]] = set()

    def _on_service_state_change(zeroconf, service_type, name, state_change):
        if state_change != ServiceStateChange.Added:
            return
        info = zeroconf.get_service_info(service_type, name)
        if not info:
            return
        for address in info.parsed_addresses():
            results.add((address, name))

    browser = ServiceBrowser(zeroconf, "_http._tcp.local.", handlers=[_on_service_state_change])
    time.sleep(timeout_seconds)
    zeroconf.close()
    return results


def scan_subnet(timeout_seconds: float = DEFAULT_TIMEOUT_SECONDS) -> List[str]:
    local_ip = _get_local_ip()
    if not local_ip:
        return []

    base = ".".join(local_ip.split(".")[:-1])
    targets = [f"{base}.{i}" for i in range(1, 255)]

    found: List[str] = []
    with ThreadPoolExecutor(max_workers=32) as executor:
        futures = {
            executor.submit(_probe_status, ip, timeout_seconds): ip for ip in targets
        }
        for future in as_completed(futures):
            ip = futures[future]
            try:
                ok = future.result()
                if ok:
                    found.append(ip)
            except Exception:
                continue

    return found


def crawl_peers(
    manager: HostManager,
    seed_host: str,
    timeout_seconds: float = DEFAULT_TIMEOUT_SECONDS,
) -> None:
    status = _get_status(seed_host, timeout_seconds)
    peers = status.get("peers", []) if isinstance(status, dict) else []
    for ip, hostname in _extract_peer_ips(peers):
        record = identify_host(ip, timeout_seconds=timeout_seconds)
        manager.add_host(
            ip,
            record.get("hostname", hostname or ip),
            record.get("description", ""),
        )


def identify_host(ip: str, timeout_seconds: float = DEFAULT_TIMEOUT_SECONDS) -> Dict[str, str]:
    status = _get_status(ip, timeout_seconds)
    if not isinstance(status, dict):
        return {"hostname": ip, "description": ""}

    hostname = str(status.get("hostname") or status.get("device") or ip)
    description = str(status.get("description") or status.get("clusterName") or "")
    return {"hostname": hostname, "description": description}


def _get_status(ip: str, timeout_seconds: float) -> Dict[str, object]:
    url = f"http://{ip}/api/status"
    try:
        response = requests.get(url, timeout=timeout_seconds)
        response.raise_for_status()
        return response.json()
    except Exception:
        return {}


def _extract_peer_ips(peers: List[object]) -> Set[Tuple[str, str]]:
    results: Set[Tuple[str, str]] = set()
    for peer in peers:
        if isinstance(peer, dict):
            ip = str(peer.get("ip") or peer.get("address") or "")
            hostname = str(peer.get("hostname") or peer.get("name") or "")
            if ip:
                results.add((ip, hostname))
        elif isinstance(peer, str):
            results.add((peer, ""))
    return results


def _probe_status(ip: str, timeout_seconds: float) -> bool:
    url = f"http://{ip}/api/status"
    try:
        response = requests.get(url, timeout=timeout_seconds)
        return response.ok
    except Exception:
        return False


def _get_local_ip() -> Optional[str]:
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            sock.connect(("8.8.8.8", 80))
            return sock.getsockname()[0]
    except Exception:
        try:
            return socket.gethostbyname(socket.gethostname())
        except Exception:
            return None
