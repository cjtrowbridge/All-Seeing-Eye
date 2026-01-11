// All-Seeing Eye - Base Plate
// Version: 1.1 
// Changes: Widen plate to match full base_width (bottom mount). Taller tabs to compensate.

/* [Global Dimensions] */
INCH = 25.4;

base_width = 8.0 * INCH;
plate_thickness = 2.0;
wall_thickness = 2.0;

// Alignment Tabs
// Plate is external. Tabs project up into the shell.
// Shell block starts at 0.25 inch.
tab_height = 0.25 * INCH; 

total_height = plate_thickness + tab_height;

// Magnet logic
magnet_diam = 6.2; // 6mm + tolerance
magnet_load_depth = total_height - 1.0; // Hole depth from bottom, leaving 1mm bridge at top

/* [Rendering] */
$fn = 60;

module base_plate_main() {
    
    // Plate is now full width, so no tolerance needed relative to wall INTERIOR.
    // However, we might want a tiny tolerance if it needs to fit... 
    // but user asked for "full width of the pyramid's outer edge".
    plate_w = base_width;
    
    difference() {
        // 1. Positive Volume
        union() {
            // A. Base Plate (Full Width)
            translate([0, 0, plate_thickness/2])
                cube([plate_w, plate_w, plate_thickness], center=true);
            
            // B. Alignment Tabs (Corner Triangles)
            // They sit on top of the plate (starts at z = plate_thickness)
            translate([0, 0, plate_thickness])
                alignment_tabs();
        }

        // 2. Negative Volume (Magnet Holes)
        // Access from BOTTOM (z=0)
        magnet_holes();
    }
}

module alignment_tabs() {
    // Generate 4 corner triangles
    // They fit INSIDE the shell walls.
    
    // Offset must still be calculated based on the INTERIOR wall position
    offset_dist = base_width/2 - wall_thickness; // Corner of the inner void
    
    // Triangle size
    tri_leg = 2.0 * INCH;
    
    // Corner 1 (+X, +Y)
    translate([offset_dist, offset_dist, 0]) rotate([0,0,180]) corner_triangle(tri_leg, tab_height);
    
    // Corner 2 (-X, +Y)
    translate([-offset_dist, offset_dist, 0]) rotate([0,0,-90]) corner_triangle(tri_leg, tab_height);
    
    // Corner 3 (-X, -Y)
    translate([-offset_dist, -offset_dist, 0]) rotate([0,0,0]) corner_triangle(tri_leg, tab_height);
    
    // Corner 4 (+X, -Y)
    translate([offset_dist, -offset_dist, 0]) rotate([0,0,90]) corner_triangle(tri_leg, tab_height);
}

module corner_triangle(leg, h) {
    // Right isosceles triangle in the corner
    linear_extrude(h) {
        polygon([
            [0,0], 
            [leg, 0], 
            [0, leg]
        ]);
    }
}

module magnet_holes() {
    // Positions must match the center of mass of the triangles or close to the corner
    offset_dist = base_width/2 - wall_thickness; 
    tri_leg = 2.0 * INCH;
    
    // Magnet position
    mag_offset = 0.75 * INCH;
    
    // Calculate global positions relative to center
    px = offset_dist - mag_offset;
    py = offset_dist - mag_offset;
    
    hole_depth = magnet_load_depth;
    
    positions = [
        [px, py],   // Top Right
        [-px, py],  // Top Left
        [-px, -py], // Bottom Left
        [px, -py]   // Bottom Right
    ];
    
    for(pos = positions) {
        translate([pos[0], pos[1], -0.1])
            cylinder(h = hole_depth + 0.1, d = magnet_diam); // +0.1 for clean cut
    }
}

base_plate_main();
