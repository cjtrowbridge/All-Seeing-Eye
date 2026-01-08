# All-Seeing-Eye Design Document
A 3d printed structure to contain broad spectrum sub-ghz meshtastic VLBI elements


## 1. Broader System Architecture

The overall system is composed of two primary node types:

* **SDR Node**

  * Performs wideband spectrum sensing
  * Handles coordination and task delegation
  * Synthesizes composite spectrum and RSSI bitmaps

* **All-Seeing Eye** Nodes (ESP32 + CC1101)

  * Perform distributed, narrowband RSSI sensing
  * Contribute localized observations to the global view

By correlating RSSI bitmaps from multiple perspectives, the system can simultaneously determine **where many RF broadcasts each originate from, in real time**. Complete spectral visibility emerges from the correlation of many limited observers.

This design document focuses on the **All-Seeing Eye Node** as a physical and conceptual artifact within this broader architecture.

---

## 2. Naming Decisions

* **All-Seeing Eye** — distributed RF observer

The term *All-Seeing Eye* is used **literally and technically**, describing emergent total visibility of the RF spectrum derived from correlated partial observations, not from a single privileged vantage point.

---

## 3. Physical Design Specifications (Prototype)

For the initial prototype, the enclosure must meet the following precise physical specifications to be modeled in 3D:

### 3.1 Base Geometry
*   **Structure**: A hollow square prism.
*   **Dimensions**: 4 inches x 4 inches (Outer footprint).
*   **Height**: 0.5 inches.
*   **Wall Thickness**: 0.25 inches.
*   **Notes**: This forms the foundation upon which the pyramid sits.

### 3.2 Pyramid Geometry
*   **Structure**: A hollow 4-sided pyramid sitting directly atop the base.
*   **Slope**: Faces must be angled at 45 degrees.
*   **Wall Thickness**: 0.25 inches.
*   **Front Face**: Reserved for future illuminated "All-Seeing Eye" symbol (not present in prototype).
*   **Top**: Truncated flat top.
    *   The flat top must be large enough to accommodate a central SMA connector hole.
    *   Ensure sufficient flat area for the connector nut (internal) and fittings (external).

### 3.3 Clearances
*   The entire assembly is hollow.
*   Internal thickness of walls is consistent (0.25 inch).

---

## 4. Connectivity & Interface Layout

The connector ports are distributed across the faces of the **Base Prism** and the **Pyramid Top**.

### 4.1 Top Interface (Pyramid Apex)
*   **Connector**: VNA / Sensing SMA.
*   **Placement**: Centered on the flat, truncated top of the pyramid.
*   **Requirements**: Through-hole sized for standard SMA bulkhead. Clearance for cable attachment.

### 4.2 Base Interfaces
The base prism has four faces. The layout is as follows:

*   **Front Face**: 
    *   **Status**: Blank / Uninterrupted.
*   **Rear Face**:
    *   **Connector**: USB / Power.
    *   **Details**: A cutout or hole reaching the bottom edge of the base, allowing a USB cable to pass under/into the unit for power and control.
*   **Left Face**:
    *   **Connector**: 2.4GHz Wi-Fi Antenna (SMA).
    *   **Details**: Through-hole for SMA bulkhead. Ensure internal clearance for the nut and external clearance for antenna rotation.
*   **Right Face**:
    *   **Connector**: Meshtastic Node Antenna (SMA).
    *   **Details**: Through-hole for SMA bulkhead. Ensure internal clearance for the nut and external clearance for antenna rotation.

This layout separates sensing from networking both **electrically and visually**, while maximizing RF isolation within a compact enclosure.

---

## 5. Lighting Design

Lighting is used to convey **latent awareness**, not alerts or activity.

### Principles

* Dim, indirect, constant illumination
* No flashing, blinking, or RGB cycling
* Light is a side effect of perception, not a signal

### Lighting Layers

* **Eye Glow**: Very low-level internal glow behind the eye aperture
* **Edge Leakage**: Subtle light escaping along seams or edges
* **Base Halo (optional)**: Soft downward glow to create a floating artifact effect

Lighting behavior is slow, restrained, and continuous, reinforcing the sense that the node is always perceiving.

---

## 6. Overall Intent

The All-Seeing Eye Node is designed as:

* a sensing instrument
* a symbolic artifact
* a field-deployable system component

The system observes **broadcast signals, not people**. Power derives from distribution, correlation, and persistence rather than authority or control.

The object is meant to feel present, inevitable, and quietly capable — something you notice after it has already been there.

---

## 7. Locked Concepts

* **Names**: SDR Node, All-Seeing Eye Node, All-Seeing Eye View
* **Form**: Pyramidal structure on a raised square base.
* **Function**: Distributed RF visibility through correlated RSSI sensing
* **Aesthetic**: Ominous, restrained, archetypal, intentional
