from __future__ import annotations

import json
from typing import Any, Dict, Optional

import requests


class EyeClient:
    def __init__(self, address: str, timeout_seconds: float = 5.0) -> None:
        self.address = address
        self.timeout_seconds = timeout_seconds

    def request(self, method: str, endpoint: str, payload: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        url = self._resolve_url(endpoint)
        try:
            response = requests.request(
                method=method.upper(),
                url=url,
                json=payload,
                timeout=self.timeout_seconds,
            )
        except requests.exceptions.ConnectionError as exc:
            return {
                "ok": False,
                "status_code": 0,
                "error": "HostUnreachable",
                "details": str(exc),
            }

        try:
            data = response.json()
        except json.JSONDecodeError:
            data = response.text

        return {
            "ok": response.ok,
            "status_code": response.status_code,
            "data": data,
        }

    def get(self, endpoint: str) -> Dict[str, Any]:
        return self.request("GET", endpoint)

    def post(self, endpoint: str, payload: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        return self.request("POST", endpoint, payload=payload)

    def _resolve_url(self, endpoint: str) -> str:
        normalized = endpoint if endpoint.startswith("/") else f"/{endpoint}"
        return f"http://{self.address}{normalized}"
