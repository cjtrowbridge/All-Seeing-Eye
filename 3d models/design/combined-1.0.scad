// All-Seeing Eye - Combined Variant
// Version: 1.0
// Description: Main pyramid shell with the eye symbol attached in place (Single print model).

use <all-seeing-eye.1.6.scad>
use <eye-symbol.1.0.scad>

/* [Global Dimensions] */
INCH = 25.4;
base_width = 8.0 * INCH;
base_height = 0.5 * INCH;

/* [Rendering] */
$fn = 60;

module main() {
    union() {
        // 1. Main Shell
        shell_main();
        
        // 2. Eye Symbol
        // Positioning logic derived from assembly.1.0.scad
        
        // Target height on the face
        place_z = 2.0 * INCH;
        
        // Calculate Y position on the 45-degree slope
        // Face starts at Y = -base_width/2 (at Z=base_height)
        // Slope goes +Y as Z increases.
        // Y = Y_start + (Z - Z_start)
        place_y = -(base_width/2) + (place_z - base_height);
        
        translate([0, place_y, place_z])
            rotate([45, 0, 0])
            eye_symbol_main();
    }
}

main();
