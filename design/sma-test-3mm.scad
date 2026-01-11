// SMA Connector Thickness Test Plate
// Purpose: Verify if 3mm wall thickness allows SMA connector nut engagement.

/* [Dimensions] */
// User specified: 2cm x 2cm square, 3mm thick.
// Using specific millimeter values for the test, but keeping INCH constant for hole sizing consistency
INCH = 25.4;

width = 20;     // 2cm
length = 20;    // 2cm
thickness = 3;  // 3mm

/* [Hole Dimensions] */
// From main project (all-seeing-eye.1.6.scad): sma_hole_diameter = 0.26 * INCH
sma_hole_diameter = 0.26 * INCH; 

/* [Rendering resolution] */
$fn = 60;

module test_plate() {
    difference() {
        // 1. Positive volume (The Plate)
        // Centered on X/Y, sitting on Z=0
        translate([0, 0, thickness/2])
            cube([width, length, thickness], center=true);
        
        // 2. Negative volume (SMA Hole)
        // Cylinder centered at origin, extending through the plate
        translate([0, 0, -1])
            cylinder(h = thickness + 2, d = sma_hole_diameter);
    }
}

test_plate();
