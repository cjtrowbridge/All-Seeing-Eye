// All-Seeing Eye - Base Plate
// Version: 1.0 (Corresponds to Shell v1.6)

/* [Global Dimensions] */
INCH = 25.4;

base_width = 8.0 * INCH;
plate_thickness = 0.25 * INCH;
wall_thickness = 0.25 * INCH;

// Alignment Tabs
tab_height = 0.25 * INCH; // Stacks on top of plate
total_height = plate_thickness + tab_height;

// Magnet logic
magnet_diam = 6.2; // 6mm + tolerance
magnet_load_depth = total_height - 1.0; // Hole depth from bottom, leaving 1mm bridge at top

/* [Rendering] */
$fn = 60;

module base_plate_main() {
    
    // Fit tolerance
    tol = 0.5; // mm gap total (0.25 per side)
    
    plate_w = base_width - 2*wall_thickness - tol;
    
    difference() {
        // 1. Positive Volume
        union() {
            // A. Base Plate (Fits INSIDE the shell walls)
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
    // The plate is ALREADY inside the walls. 
    // So the tabs are flush with the plate corners?
    // Let's keep them aligned to the same coordinate system.
    
    offset_dist = base_width/2 - wall_thickness; // Corner of the inner void
    
    // Triangle size: Fill the corner? 
    // Let's make them 1 inch legs for stability
    tri_leg = 1.0 * INCH;
    
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
    tri_leg = 1.0 * INCH;
    
    // Magnet position: centered in the triangle's bulk?
    // Let's place it at (leg/3, leg/3) from the inner corner (Center of mass of triangle is 1/3)
    // Actually, closer to corner is better for snapping. Let's do 0.25 inch from corner.
    mag_offset = 0.25 * INCH;
    
    // Calculate global positions relative to center
    // Inner corner is at +/- offset_dist
    // We want to move 'inward' by mag_offset
    
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
