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

// Calculate pyramid height
pyramid_height = (base_width - top_width) / 2;

/* [Interface Dimensions] */
sma_hole_diameter = 0.26 * INCH;
usb_cutout_width = 0.6 * INCH;
usb_cutout_height = 0.3 * INCH;
usb_cable_clearance = 0.5 * INCH;

// ESP32-S3 Board Dimensions
pcb_width = 28.0; 
pcb_length = 58.0; 
pcb_thickness = 1.6; 
pin_row_clearance = 2.54; // Standard pin header height ~2.54mm or more, keep clear

// LED Position (offset from center of board)
// Board center X is 14mm. LED center X is 8mm.
// Offset = 8 - 14 = -6mm (Left).
led_offset_x = -6.0; 
led_center_y_from_bottom = 18.0; 
led_size = 6.0;

/* [Rendering resolution] */
$fn = 60;

module main() {
    difference() {
        // 1. Positive Volume (The Shell + Mounts)
        union() {
            // A. Base Prism
            translate([0, 0, base_height/2])
                cube([base_width, base_width, base_height], center=true);
            
            // B. Pyramid Frustum
            translate([0, 0, base_height])
                pyramid_frustum(base_width, top_width, pyramid_height);
            
            // C. Internal Mounts (Positive Geometry)
            // CALCULATE POSITION:
            // 1. Z-Height of Eye Center
            eye_z_center = base_height + pyramid_height * 0.4;
            
            // 2. Y-Pos on the Inner Wall Surface.
            // Pyramid Inner Wall equation for -Y face: Y = -(base_width/2 - wall_thickness) + (Z - base_height)
            // Wait, wall thickness is subtracted from all sides.
            // So inner base edge starts at Y = -(base_width/2 - wall_thickness).
            // Let's verify slope direction. Z goes up, Y goes towards 0 (inwards).
            // So Y = -(Inner_Base_Radius) + (Z_Height_Above_Base)
            inner_base_radius = (base_width/2) - wall_thickness;
            eye_y_inner = -inner_base_radius + (eye_z_center - base_height);

            translate([0, eye_y_inner, eye_z_center])
                rotate([45, 0, 0]) // Match face slope
                // No need to rotate 180 here, let's build the module "Face Down" natively.
                // Origin of module = LED Center on the Face surface.
                esp32_mount_assembly();
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
            
        // E. Front Face Eye Hole
        // Recalculate position same as above
        eye_z = base_height + pyramid_height * 0.4;
        eye_y = -base_width/2 + (eye_z - base_height);
        
        translate([0, eye_y, eye_z])
            rotate([45, 0, 0]) 
            // We want the hole to be where the LED is.
            // Our mount is centered on X=0.
            // The Board is centered in the mount.
            // The LED is offset by led_offset_x on the board.
            // BUT wait, we flipped the board upside down (rotate 180 Y or X).
            // If we flip X, Left becomes Right.
            // If we flip Y (as done above with Z local), Right is still Right (looking from top).
            // Let's assume standard view:
            // Hole needs to be exactly where the mount puts the LED.
            translate([-led_offset_x, 0, 0]) // Shift hole to match LED offset
                cylinder(h = wall_thickness * 3, d = led_size, center = true);
    }
}

// Module for the specific ESP32 holder geometry
module esp32_mount_assembly() {
    // This module is positioned at the "Eye Center" on the INNER face surface.
    // Orientation: Local Z points INWARDS (towards center of pyramid).
    // Local X/Y plane is parallel to the 45-degree wall.
    // Origin (0,0) is the LED Center.
    
    // Board Orientation: "Face Down" means PCB Top (LED side) touches the wall (Z=0).
    // The board itself extends into +Z space.
    
    // We need Rails to hold the EDGES of the PCB.
    // PCB Width: 28mm.
    // Rails should be at X = +/- 14mm.
    
    // We need to shift the geometry relative to the LED Center Origin.
    // LED is at X = led_offset_x = -6mm (relative to PCB Center).
    // So PCB Center is at X = -led_offset_x = +6mm relative to LED/Origin.
    
    // LED is at Y = led_center_y_from_bottom = 18mm (relative to PCB Bottom).
    // So PCB Bottom is at Y = -18mm relative to LED/Origin.
    
    pcb_center_x_local = -led_offset_x; // +6mm
    pcb_bottom_y_local = -led_center_y_from_bottom; // -18mm
    
    rail_height = 8.0; // How far off the wall they stick out
    rail_width = 3.0;
    
    translate([pcb_center_x_local, pcb_bottom_y_local, 0]) {
        // Now we are at the Bottom-Center of the PCB footprint (virtually).
        
        // 1. Left Rail (at X = -14)
        translate([-pcb_width/2 - rail_width/2, pcb_length/2, rail_height/2])
            cube([rail_width, pcb_length, rail_height], center=true);

        // 2. Right Rail (at X = +14)
        translate([pcb_width/2 + rail_width/2, pcb_length/2, rail_height/2])
            cube([rail_width, pcb_length, rail_height], center=true);
            
        // 3. Top Stop (Antenna End)
        // Keeps board from sliding "Up" (Interference fit or stop)
        translate([0, pcb_length + 1, rail_height/2])
            cube([pcb_width + 2*rail_width, 2, rail_height], center=true);
            
        // 4. "Hook" / Pin-Clearance Shelf
        // You mentioned: "hook can rest between the pins... holding the board... two raised rails rest on outside"
        // If we are Face Down, the rails are holding the SIDES.
        // Something needs to press the BACK of the board (the non-LED side) towards the wall?
        // Or if the rails are "L" shaped to trap it?
        // Let's make "L" shaped rails.
        
        // Left Overhang
        translate([-pcb_width/2 - 1, pcb_length/2, rail_height]) // Top of rail
            cube([4, pcb_length, 2], center=true); // 2mm overhang tab
            
        // Right Overhang
        translate([pcb_width/2 + 1, pcb_length/2, rail_height]) 
            cube([4, pcb_length, 2], center=true); 
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
