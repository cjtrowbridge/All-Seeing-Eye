# Playbook: Auditing Cluster Status and Peer Discovery

*Status: Draft*

## Objective
This playbook covers how to use the Python client and API endpoints to audit the health of the entire cluster, verify that all nodes see each other (Mesh integrity), and ensure consistency of the "shared reality" (Task distribution).

## Prerequisites
*   Python 3 installed with dependencies (`requests`, `zeroconf`).
*   Scripts located in `/api/`.
*   At least 2 running nodes.

## Step-by-Step Instructions

1.  **Run the Cluster Auditor Script**
    *   *Note*: If specific script `audit_cluster.py` does not exist, use `check_status.py` or write a script using the `client.py` library.
    *   **Command**: `python api/check_status.py`
    *   **Goal**: Get a JSON dump of every node's status.

2.  **Analyze Peer Lists (The "Mesh" View)**
    *   **Action**: For each node, look at its `peers` list.
    *   **Check**: Does Node A see Node B? Does Node B see Node A?
    *   **Split Brain**: If Node A sees {B, C} but Node D sees {E, F}, you have a network partition (Split Brain) or mDNS failure.

3.  **Verify Task Distribution**
    *   **Action**: internal field `task`.
    *   **Check**: Are all nodes reporting "Independent Exploration" (Idle)? Or are they stuck in old tasks?
    *   **Fix**: If a node reports a task that doesn't exist, use the header/API to `POST /api/task { "id": "idle" }`.

4.  **Check Signal Strength (RSSI)**
    *   **Metric**: `rssi` field in peer objects.
    *   **Acceptable**: > -75 dBm.
    *   **Poor**: < -85 dBm.
    *   **Action**: If RSSI is poor, move nodes closer or check antenna connections (SMA).

## Verification
*   All nodes are accounted for in the audit.
*   Every node's peer count matches `(Total Nodes - 1)`.
*   No nodes are in error states.
