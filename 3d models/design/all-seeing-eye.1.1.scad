// All-Seeing Eye Enclosure Design
// Unit: mm (Converted from freedom units)

/* [Global Dimensions] */
INCH = 25.4;

// Base Specifications
base_width = 8.0 * INCH;
base_height = 0.5 * INCH;
wall_thickness = 0.25 * INCH;

// Pyramid Specifications
// Slope is 45 degrees.
// For a 45 degree slope, the change in height equals the change in radius (distance from center).
// Height to Apex = (base_width / 2) * tan(45) = base_width / 2
slope_angle = 45; 

// Top flat area for SMA connector
top_width = 1.0 * INCH; 

// Calculate pyramid height based on slope and top width
// The horizontal run from base edge to top edge is: (base_width - top_width) / 2
// height = run * tan(angle). Since angle is 45, tan is 1.
pyramid_height = (base_width - top_width) / 2;

// Connector Dimensions
sma_hole_diameter = 0.26 * INCH; // ~6.6mm, purely clearance for SMA thread
usb_cutout_width = 0.6 * INCH;
usb_cutout_height = 0.3 * INCH; // Height of the actual hole
usb_cable_clearance = 0.5 * INCH; // Width of slot at bottom

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
            // Positioned on top of the base
            translate([0, 0, base_height])
                pyramid_frustum(base_width, top_width, pyramid_height);
        }

        // 2. Negative Volume (The Hollow)
        union() {
            // A. Hollow Base
            // Subtract thickness from X and Y, but allow Z to cut through to pyramid
            translate([0, 0, base_height/2])
                cube([base_width - 2*wall_thickness, base_width - 2*wall_thickness, base_height + 0.1], center=true);
            
            // B. Hollow Pyramid
            // To maintain roughly constant wall thickness perpendicular to a 45 deg slope is tricky with simple scaling.
            // Simple approach: Subtract inner pyramid starting at same height, but with smaller base.
            // Inner base width = Outer Width - 2 * wall_thickness
            // We want the inner void to match up with the base void.
            inner_base_width = base_width - 2*wall_thickness;
            inner_top_width = max(0, top_width - 2*wall_thickness); // Ensure it doesn't invert
            
            // The height of the inner pyramid needs to be calculated or offset carefully.
            // Since it's 45 degrees, the inner ceiling is lower by 'wall_thickness'.
            translate([0, 0, base_height])
                 pyramid_frustum(inner_base_width, inner_top_width, pyramid_height);
        }

        // 3. Cutouts / Holes
        
        // A. Top SMA Connector (Apex)
        translate([0, 0, base_height + pyramid_height - 1]) // Start slightly below top
            cylinder(h = wall_thickness + 2, d = sma_hole_diameter, center = false);

        // B. Left Face SMA (WiFi) - Looking from Front
        // Front is -Y (standard convention often, but let's define Front).
        // Let's assume:
        // +Y = Back (Rear)
        // -Y = Front
        // -X = Left
        // +X = Right
        
        // Left Face (-X)
        translate([-base_width/2 + wall_thickness/2, 0, base_height/2])
            rotate([0, 90, 0])
            cylinder(h = wall_thickness + 2, d = sma_hole_diameter, center = true);

        // C. Right Face SMA (Meshtastic) (+X)
        translate([base_width/2 - wall_thickness/2, 0, base_height/2])
            rotate([0, 90, 0])
            cylinder(h = wall_thickness + 2, d = sma_hole_diameter, center = true);

        // D. Rear Face USB (+Y)
        // "Hole reaching to the bottom of the base"
        translate([0, base_width/2 - wall_thickness/2, 0]) // At bottom Z=0
            cube([usb_cutout_width, wall_thickness + 2, base_height*1.5], center = true);
            
    }
}

// Module to create a centered 4-sided pyramid frustum
module pyramid_frustum(base_w, top_w, h) {
    // OpenSCAD's cylinder with $fn=4 creates a square rotated by 45 degrees.
    // To align it with axes, we rotate it by 45 degrees.
    // circumradius = side / sqrt(2) * 2? No.
    // side = r * sqrt(2). r = side / sqrt(2).
    
    r_base = (base_w / 2) * sqrt(2);
    r_top = (top_w / 2) * sqrt(2);
    
    rotate([0, 0, 45])
        cylinder(h=h, r1=r_base, r2=r_top, $fn=4);
}

main();
