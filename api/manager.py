from __future__ import annotations

import csv
import sys
import time
from pathlib import Path
from typing import Dict, List, Optional
import shutil

CSV_HEADERS = ["ip_address", "hostname", "description", "last_seen_timestamp"]


class HostManager:
    def __init__(self, csv_path: Optional[Path] = None) -> None:
        self.csv_path = csv_path or Path(__file__).resolve().parent / "known_hosts.csv"

    def load_hosts(self) -> List[Dict[str, str]]:
        if not self.csv_path.exists():
            return []

        try:
            with self.csv_path.open("r", newline="", encoding="utf-8") as file:
                reader = csv.DictReader(file)
                if reader.fieldnames != CSV_HEADERS:
                    raise ValueError(f"Unexpected headers: {reader.fieldnames}")
                return [self._normalize_row(row) for row in reader]
        except Exception as exc:
            print(f"[HostManager] Warning: invalid known_hosts.csv: {exc}", file=sys.stderr)
            self._backup_corrupt_file()
            return []

    def save_hosts(self, hosts: List[Dict[str, str]]) -> None:
        with self.csv_path.open("w", newline="", encoding="utf-8") as file:
            writer = csv.DictWriter(file, fieldnames=CSV_HEADERS)
            writer.writeheader()
            for row in hosts:
                writer.writerow(self._normalize_row(row))

    def add_host(self, ip: str, hostname: str, description: Optional[str] = None) -> None:
        hosts = self.load_hosts()
        updated = False
        for host in hosts:
            if host["ip_address"] == ip:
                host["hostname"] = hostname or host["hostname"]
                if description:
                    host["description"] = description
                host["last_seen_timestamp"] = str(int(time.time()))
                updated = True
                break

        if not updated:
            hosts.append(
                {
                    "ip_address": ip,
                    "hostname": hostname,
                    "description": description or "",
                    "last_seen_timestamp": str(int(time.time())),
                }
            )

        self.save_hosts(hosts)

    def resolve_host(self, query: Optional[str]) -> Dict[str, str]:
        hosts = self.load_hosts()
        if not hosts:
            raise ValueError("No known hosts available.")

        if query is None:
            return hosts[0]

        for host in hosts:
            if query == host.get("ip_address") or query == host.get("hostname"):
                return host

        for host in hosts:
            if query == host.get("description") and host.get("description"):
                return host

        for host in hosts:
            if query.lower() in host.get("hostname", "").lower():
                return host

        for host in hosts:
            if query.lower() in host.get("description", "").lower():
                return host

        raise ValueError(
            "Host '{query}' not found in known_hosts.csv. "
            "Use IP address, hostname, or description."
        )

    def list_hosts(self) -> List[Dict[str, str]]:
        return self.load_hosts()

    def _normalize_row(self, row: Dict[str, str]) -> Dict[str, str]:
        return {header: row.get(header, "") for header in CSV_HEADERS}

    def _backup_corrupt_file(self) -> None:
        if not self.csv_path.exists():
            return
        backup_path = self.csv_path.with_suffix(".csv.bak")
        try:
            shutil.copy(self.csv_path, backup_path)
        except Exception as exc:
            print(f"[HostManager] Warning: failed to backup CSV: {exc}", file=sys.stderr)
