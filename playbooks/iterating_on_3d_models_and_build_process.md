# Playbook: Iterating on 3D Models and Build Process

*Status: Stable*

## Objective
This playbook explains how to modify the physical design of the Eye, rebuild the STL files using OpenSCAD, and update the repository artifacts.

## Prerequisites
*   OpenSCAD installed (and added to PATH, or standard install location).
*   `3d models/` directory structure.

## Step-by-Step Instructions

1.  **Modify Source Files (Versioning)**
    *   **Location**: `3d models/design/*.scad`.
    *   **Rule**: **Do not overwrite existing files**. Always save your changes as a new minor revision.
        *   *Example*: Copy `all-seeing-eye.1.6.scad` to `all-seeing-eye.1.7.scad` before making changes.
    *   **Action**: Edit the new SCAD file. Use `assembly.1.0.scad` (updated to point to your new file) to visualize how parts fit together.

2.  **Generate STLs (Build)**
    *   **Context**: Open terminal in `3d models/`.
    *   **Command**: `.\build.bat`
    *   **Process**: The script finds OpenSCAD and compiles every `.scad` file in `design/` to an `.stl` file in `build/`.

3.  **Inspect Artifacts**
    *   **Location**: `3d models/build/`.
    *   **Action**: Open the new STLs in a slicer (Cura/PrusaSlicer) or 3D viewer to check for manifold errors or geometry issues.

4.  **Commit Changes**
    *   Commit both the source (`.scad`) and the binary artifacts (`.stl`). This allows people without OpenSCAD to print immediately.

## Verification
*   Date modified of files in `build/` matches current time.
*   File size of STLs is non-zero.
*   Physical print matches design intent.
