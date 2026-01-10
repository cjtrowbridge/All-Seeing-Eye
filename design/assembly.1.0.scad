// All-Seeing Eye - Full Assembly Visualization
// Version: 1.0
// Description: Renders all components (Shell, Base Plate, Eye Symbol) in their assembled positions.

use <all-seeing-eye.1.6.scad>
use <base-plate.1.0.scad>
use <eye-symbol.1.0.scad>

INCH = 25.4;
$fn = 60;

// Dimensions needed for placement calculation
base_width = 8.0 * INCH;
base_height = 0.5 * INCH;
slope_angle = 45; 

// Explosion Factor: 0 = fully assembled, >0 = exploded view
explode = 10; 

module assembly() {
    
    // 1. Shell (Main Body)
    // Sits on top of the base plate (Z=0 if plate is flush, or Z=plate_thick if resting?)
    // In our design, the base plate fits INSIDE the shell walls, but sits at the bottom.
    // The Shell's "Base Prism" starts at Z=0.
    // So we render the shell at Z=0 + explode.
    translate([0, 0, explode])
        shell_main();

    // 2. Base Plate (Bottom Insert)
    // Sits at Z=0.
    // It fits inside the shell's hollow base.
    translate([0, 0, -explode])
        base_plate_main();

    // 3. Eye Symbol (Attached to Front Face)
    // Front face is facing -Y (in standard orientation? check shell code).
    // Shell code: Rear Face USB is usually at +Y or -Y.
    // Shell Code: 
    //   Rear Face USB: translate([0, base_width/2 ...]) -> +Y
    //   Left Face WiFi: -X
    //   Right Face Mesh: +X
    //   So "Front Face" is -Y.
    
    // Geometry:
    // Face starts at Y = -base_width/2.
    // Face slopes UP and IN toward +Y.
    // Slope is 45 degrees.
    // Center of base face: [0, -4.0 inch, 0.5 inch] (Top of base prism).
    // The pyramid face starts there.
    
    // Placement:
    // We want it centered on the pyramid face.
    // Midpoint of pyramid face height?
    // Pyramid height is ~3.5 inch. Midpoint Z ~ 0.5 + 1.75 = 2.25 inch.
    // At Z=2.25, Y is closer to center.
    // Horizontal distance from edge = Height * tan(45) = Height.
    // So Y position = -base_width/2 + (Z_pos - Base_Height).
    
    // Let's target Z = 2.0 inch (arbitrary aesthetic center).
    place_z = 2.0 * INCH;
    place_y = -(base_width/2) + (place_z - base_height); 
    
    // Rotation:
    // Face slopes 45 degrees.
    // We need to rotate the eye so its back lies on the plane.
    // The eye is created flat on XY plane.
    // Rotating around X axis:
    // +X rotation brings +Y up towards +Z.
    // -X rotation brings +Y down towards -Z.
    // We want the eye standing up (90 deg) then tilted back (45 deg).
    // Or just tilted 45?
    // Face normal vector is [0, -1, 1] (Pointing South-Up).
    // Eye normal is [0, 0, 1].
    // To align [0,0,1] to [0,-1,1], we rotate around X.
    // Angle = -45 degrees? (Check direction).
    // If I rotate -45, top of eye goes away (North), bottom comes near.
    // Wait, Front Face is -Y.
    // So we want the eye to face -Y and +Z.
    // Rotation X = -45 means Y becomes (Y cos - Z sin), Z becomes (Y sin + Z cos).
    // Actually, simple logic:
    // Start Vertical: Rotate [90, 0, 0]. Eye faces -Y. Top is +Z.
    // Tilt Back: Rotate [-45, 0, 0] relative to that?
    // Result: Rotate [90 - 45, 0, 0] = [45, 0, 0].
    
    eye_pos = [0, place_y, place_z];
    
    // Add explode vector (move away from center normal)
    explode_vec = [0, -1, 1] / norm([0,-1,1]) * explode;
    
    translate(eye_pos + explode_vec)
        rotate([45, 0, 0]) // Correct rotation to align +Z face with -Y/+Z slope
        eye_symbol_main();
}

assembly();
