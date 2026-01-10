// All-Seeing Eye Enclosure Design
// Version: 1.6
// Changes: Added corner magnet blocks to mate with Base Plate v1.0

/* [Global Dimensions] */
INCH = 25.4;

// Base Specifications
base_width = 8.0 * INCH;
base_height = 0.5 * INCH; // The vertical rim before slope
wall_thickness = 0.25 * INCH;

// Pyramid Specifications
slope_angle = 45; 
top_width = 1.0 * INCH; 
pyramid_height = (base_width - top_width) / 2;

// Connector Dimensions
sma_hole_diameter = 0.26 * INCH;
usb_cutout_width = 0.6 * INCH;
usb_cutout_height = 0.3 * INCH;

// Magnetic Latch Specifications
tri_leg = 1.0 * INCH; // Size of corner block
// The lid tabs occupy Z=0 to Z=0.25 inside the shell.
// The shell blocks occupy Z=0.25 to Z=0.5.
block_start_z = 0.25 * INCH;
block_height = 0.25 * INCH; 
magnet_diam = 6.2;
// We drill from the top of the block (Z=0.5) downwards.
// We want the magnet close to the bottom of the block (Z=0.25).
// Bridge thickness = 1mm.
magnet_hole_depth = block_height - 1.0; 

/* [Rendering resolution] */
$fn = 60;

module main() {
    difference() {
        // 1. Positive Volume (The Shell)
        union() {
            // A. Base Prism
            translate([0, 0, base_height/2])
                cube([base_width, base_width, base_height], center=true);
            
            // B. Pyramid Frustum
            translate([0, 0, base_height])
                pyramid_frustum(base_width, top_width, pyramid_height);
        }

        // 2. Negative Volume (The Hollow)
        union() {
            difference() {
                // A. Hollow Base
                translate([0, 0, base_height/2])
                    cube([base_width - 2*wall_thickness, base_width - 2*wall_thickness, base_height + 0.1], center=true);
                
                // EXCLUDE corners from the hollow (creating solid blocks)
                // Position: Inside corners, Upper half of base_height
                if(true) { // Scope for clarity
                    offset_dist = base_width/2 - wall_thickness;
                    
                    // Blocks start at block_start_z
                    translate([0,0, block_start_z])
                        corner_blocks(tri_leg, block_height, offset_dist);
                }
            }

            // B. Hollow Pyramid
            inner_height = pyramid_height - wall_thickness;
            inner_base_width = base_width - 2*wall_thickness;
            inner_top_width = top_width;

            translate([0, 0, base_height])
                 pyramid_frustum(inner_base_width, inner_top_width, inner_height);
        }

        // 3. Cutouts / Holes
        
        // A. Top SMA Connector (Apex)
        translate([0, 0, base_height + pyramid_height - wall_thickness - 1]) 
            cylinder(h = wall_thickness + 2, d = sma_hole_diameter, center = false);

        // B. Left Face SMA (WiFi)
        translate([-base_width/2 + wall_thickness/2, 0, base_height/2])
            rotate([0, 90, 0])
            cylinder(h = wall_thickness + 2, d = sma_hole_diameter, center = true);

        // C. Right Face SMA (Meshtastic)
        translate([base_width/2 - wall_thickness/2, 0, base_height/2])
            rotate([0, 90, 0])
            cylinder(h = wall_thickness + 2, d = sma_hole_diameter, center = true);

        // D. Rear Face USB
        translate([0, base_width/2 - wall_thickness/2, 0])
            cube([usb_cutout_width, wall_thickness + 2, base_height*1.5], center = true);
            
        // E. Magnet Recesses using magnet_diam
        // Drilled into the top of the corner blocks
        magnet_recesses();
    }
}

module pyramid_frustum(base_w, top_w, h) {
    r_base = (base_w / 2) * sqrt(2);
    r_top = (top_w / 2) * sqrt(2);
    
    rotate([0, 0, 45])
        cylinder(h=h, r1=r_base, r2=r_top, $fn=4);
}

module corner_blocks(leg, h, offset_dist) {
    // Generates 4 corner triangles
    // Origin is center of assembly. 
    // This module is translated to the correct Z height in 'main'.
    
    // Corner 1 (+X, +Y)
    translate([offset_dist, offset_dist, 0]) rotate([0,0,180]) corner_triangle(leg, h);
    
    // Corner 2 (-X, +Y)
    translate([-offset_dist, offset_dist, 0]) rotate([0,0,-90]) corner_triangle(leg, h);
    
    // Corner 3 (-X, -Y)
    translate([-offset_dist, -offset_dist, 0]) rotate([0,0,0]) corner_triangle(leg, h);
    
    // Corner 4 (+X, -Y)
    translate([offset_dist, -offset_dist, 0]) rotate([0,0,90]) corner_triangle(leg, h);
}

module corner_triangle(leg, h) {
    // Extrudes UP from Z=0 local
    linear_extrude(h) {
        polygon([
            [0,0], 
            [leg, 0], 
            [0, leg]
        ]);
    }
}

module magnet_recesses() {
    offset_dist = base_width/2 - wall_thickness;
    mag_offset = 0.25 * INCH;
    
    px = offset_dist - mag_offset;
    py = offset_dist - mag_offset;
    
    // Recess starts at Top of Block (z = base_height = 0.5")
    // Goes down by magnet_hole_depth
    
    start_z = base_height + 0.1; // Slightly above for clean cut
    
    positions = [
        [px, py],
        [-px, py],
        [-px, -py],
        [px, -py]
    ];
    
    for(pos = positions) {
        translate([pos[0], pos[1], start_z])
            rotate([180, 0, 0]) // Flip to drill down
            cylinder(h = magnet_hole_depth + 0.1, d = magnet_diam);
    }
}

main();
