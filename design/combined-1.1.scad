// All-Seeing Eye - Combined Variant (Transparent Sclera)
// Version: 1.1
// Description: Main pyramid shell with the eye symbol fused, and the wall REMOVED behind the sclera for LED transparency.

use <all-seeing-eye.1.6.scad>
use <eye-symbol.1.0.scad>

/* [Global Dimensions] */
INCH = 25.4;
base_width = 8.0 * INCH;
base_height = 0.5 * INCH;

// Eye dimensions mirrored from eye-symbol.1.0.scad
eye_width = 3.0 * INCH;
eye_height = 1.75 * INCH;
// Replicating logic from eye-symbol.1.0.scad
rim_width = 0.15 * INCH;

// Placement Logic
place_z = 2.0 * INCH;
place_y = -(base_width/2) + (place_z - base_height);

/* [Rendering] */
$fn = 60;

module main() {
    union() {
        // 1. Modified Shell with Cutout
        difference() {
            shell_main();
            
            // The Transparency Cutout
            // Reduces the wall behind the "white of the eye" to air,
            // so only the eye's base_thickness (0.08") remains.
            translate([0, place_y, place_z])
                rotate([45, 0, 0])
                translate([0, 0, -1.0 * INCH]) // Start cut effectively infinitely deep inside
                    linear_extrude(1.0 * INCH + 0.1) // Extrude up to just past the surface? 
                    // We want to cut the WALL, but not the EYE (which isn't here yet).
                    // Shell surface is at Local Z=0.
                    // We cut up to Z=0.01 to ensure the surface is opened.
                    // The Eye will be placed at Z=0, sealing the hole.
                        almond_shape(eye_width - rim_width*2, eye_height - rim_width*2);
        }
        
        // 2. The Eye Symbol
        translate([0, place_y, place_z])
            rotate([45, 0, 0])
            eye_symbol_main();
    }
}

main();
