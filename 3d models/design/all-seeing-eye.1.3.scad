// All-Seeing Eye Enclosure Design
// Unit: mm (Converted from freedom units)

/* [Global Dimensions] */
INCH = 25.4;

// Base Specifications
// 8.0 inches (203.2 mm) outer width
base_width = 8.0 * INCH;
base_height = 0.5 * INCH;
wall_thickness = 0.25 * INCH;

// Pyramid Specifications
// Slope is 45 degrees.
slope_angle = 45; 

// Top flat area for SMA connector
top_width = 1.0 * INCH; 

// Calculate pyramid height based on slope and top width
// height = run * tan(angle). Since angle is 45, tan is 1.
pyramid_height = (base_width - top_width) / 2;

/* [Interface Dimensions] */
// Connector Dimensions
sma_hole_diameter = 0.26 * INCH;
usb_cutout_width = 0.6 * INCH;
usb_cutout_height = 0.3 * INCH;
usb_cable_clearance = 0.5 * INCH;

// ESP32-S3 Board Dimensions & Mounting
// PCB Physical Dimensions
pcb_width = 28.0;  // Measured: 2.8cm
pcb_length = 58.0; // Measured: 5.8cm
pcb_thickness = 1.6; // Standard PCB thickness (approx)

// LED Position Logic
// "Left side of board to right side of LED is 11mm"
// LED is 6mm wide.
// Center X = Right Edge (11) - Half Width (3) = 8mm from left edge?
// Re-read carefuly: "left side of board to right side of led is 11mm"
// So LED Right Edge is at X=11.
// LED Left Edge is at X=11-6 = 5.
// LED Center X = 5 + 3 = 8mm.
// Confirm centering: If PCB is 28mm wide... Center is 14mm.
// 8mm is definitely not centered on PCB. It's offset to left.

// "Top of LED to bottom of USB is 21mm"
// LED is 6mm tall.
// "Bottom of USB" = Bottom edge of PCB (Datum Y=0)
// "Top of LED" is at Y=21.
// LED Bottom Edge is at Y=21-6 = 15.
// LED Center Y = 15 + 3 = 18mm from bottom edge.

led_center_x = 8.0;  // From left edge
led_center_y = 18.0; // From bottom edge (USB end)
led_size = 6.0;

// Mounting Config
// The board should be mounted inside the pyramid.
// We need to place it such that the LED Center aligns with a new hole on the front face.
// Front Face is -Y face.
// The hole should be centered horizontally on the face.
// But the LED is NOT centered on the PCB (8mm vs 14mm center).
// So the PCB must be mounted OFFSET from the pyramid center line by (14 - 8) = 6mm.

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
        // Must penetrate the roof thickness.
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
            
        // E. Front Face Eye Hole (NEW)
        // Located on the -Y face.
        // Needs to clearly show the LED.
        // Let's place it at a height reasonable for the board mounting.
        // If board is mounted vertically, Y is Up. IF mounted horizontally, Z is up.
        // We haven't defined mounting orientation yet, but user wants hole on Face.
        // Let's create a placeholder hole half-way up the face for now to show intent.
        
        eye_hole_y_offset = -base_width/4; // Rough position on -Y face
        eye_hole_z_height = base_height + pyramid_height/3; 
        
        // Project a cylinder perpendicular to the 45 deg face
        translate([0, -base_width/4, eye_hole_z_height]) 
            rotate([45, 0, 0]) // Angle to match face normal roughly (check math later)
            cylinder(h = wall_thickness * 4, d = led_size * 1.5, center = true);
    }
}

// Module to create a centered 4-sided pyramid frustum
module pyramid_frustum(base_w, top_w, h) {
    r_base = (base_w / 2) * sqrt(2);
    r_top = (top_w / 2) * sqrt(2);
    
    rotate([0, 0, 45])
        cylinder(h=h, r1=r_base, r2=r_top, $fn=4);
}

main();
