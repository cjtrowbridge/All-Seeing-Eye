// All-Seeing Eye Enclosure Design
// Unit: mm (Converted from freedom units)

/* [Global Dimensions] */
INCH = 25.4;

// Base Specifications
base_width = 8.0 * INCH;
base_height = 0.5 * INCH;
wall_thickness = 0.25 * INCH;

// Pyramid Specifications
slope_angle = 45; 

// Top flat area for SMA connector
top_width = 1.0 * INCH; 

// Calculate pyramid height based on slope and top width
pyramid_height = (base_width - top_width) / 2;

// Connector Dimensions
sma_hole_diameter = 0.26 * INCH;
usb_cutout_width = 0.6 * INCH;
usb_cutout_height = 0.3 * INCH;
usb_cable_clearance = 0.5 * INCH;

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
            // A. Hollow Base
            translate([0, 0, base_height/2])
                cube([base_width - 2*wall_thickness, base_width - 2*wall_thickness, base_height + 0.1], center=true);
            
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
            
    }
}

module pyramid_frustum(base_w, top_w, h) {
    r_base = (base_w / 2) * sqrt(2);
    r_top = (top_w / 2) * sqrt(2);
    
    rotate([0, 0, 45])
        cylinder(h=h, r1=r_base, r2=r_top, $fn=4);
}

main();
