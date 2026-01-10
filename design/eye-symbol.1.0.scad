// All-Seeing Eye Symbol
// Version: 1.0
// Description: A minimalistic "lidded" eye symbol with sharp corners (Vesica Piscis geometry).

/* [Global Dimensions] */
INCH = 25.4;

eye_width = 3.0 * INCH;
eye_height = 1.75 * INCH;

base_thickness = 0.08 * INCH;  // The back plate
lid_thickness = 0.12 * INCH;   // The rim/lids
pupil_thickness = 0.12 * INCH; // The pupil

pupil_diameter = 0.9 * INCH;
iris_diameter = 1.4 * INCH;    // Optional slight detail

/* [Rendering] */
$fn = 100;

module almond_shape(w, h) {
    // To get the sharp corners (Vesica Piscis style), we intersect two large offset circles.
    // We adjust the offset and radius to approximate the bounding box w x h.
    
    // Mathematical approximation for the "Lens" shape:
    // We want the intersection to be width 'w' and height 'h'.
    // Radius of curvature 'R' = (w^2 + h^2) / (4h)
    // Vertical Offset of centers 'y0' = R - h/2
    
    R = (pow(w, 2) + pow(h, 2)) / (4 * h);
    y_offset = R - h/2;

    intersection() {
        translate([0, -y_offset, 0]) circle(r=R);
        translate([0, y_offset, 0]) circle(r=R);
    }
}

module main() {
    
    // 1. Base Plate (Sclera)
    color("white") 
    linear_extrude(base_thickness) {
        almond_shape(eye_width, eye_height);
    }

    // 2. Eyelids (Rim)
    // We create a rim by subtracting a slightly smaller almond from the main one.
    rim_width = 0.15 * INCH;
    
    color("gray")
    translate([0, 0, base_thickness])
    linear_extrude(lid_thickness - base_thickness) {
        difference() {
            almond_shape(eye_width, eye_height);
            almond_shape(eye_width - rim_width*2, eye_height - rim_width*2);
        }
    }

    // 3. Pupil & Iris
    // Independent central cylinder
    color("black")
    translate([0, 0, base_thickness])
    linear_extrude(pupil_thickness - base_thickness) {
        // Outer Iris Ring (optional, adds the "stare" effect)
        difference() {
            circle(d=iris_diameter);
            circle(d=pupil_diameter);
        }
        
        // Inner Pupil
        circle(d=pupil_diameter * 0.4); 
    }
    
    /* 
       Alternative "Heavy Lid" cut?
       Sometimes "lidded" means the top of the iris is covered.
       Uncomment below to subtract the top lid from the pupil if desired.
    */
    /*
    translate([0, eye_height/2, 0])
        cube([eye_width, eye_height*0.2, 10], center=true);
    */
}

main();
