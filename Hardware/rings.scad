overlap = 0.001;
thumb_dia = 16;
index_dia = 14;
middle_dia = 15;
ring_dia = 14;
pinky_dia = 12;

// TODO:
// - fillets

module ring(inner_diameter, tolerance=0.2, hole_angle=0){
    tol = tolerance;
    strength = 3;
    outer_diameter = inner_diameter + strength;
    thickness = 2;
    pcb_width = 13;
    pcb_height = 1;
    catch_wall = 1.5;
    catch_width = pcb_width + 2*catch_wall;
    catch_overhang = 0.5;
    prong_height = 2;
    catch_height = pcb_height + prong_height;
    
    difference(){
    union(){
    cylinder(h = thickness, r=outer_diameter/2, center=true); // Outer ring
    difference(){
    translate([-catch_width/2,outer_diameter/2,-thickness/2])cube([catch_width, catch_height, thickness]); // sense unit holder base
    
    translate([0,outer_diameter/2+(pcb_height+tol)/2, 0])cube([pcb_width+tol,pcb_height+tol,thickness], center = true);// pcb
    translate([0,outer_diameter/2+catch_height-prong_height/2-overlap, 0])cube([pcb_width-catch_overhang*2+tol,prong_height+2*overlap,thickness], center = true); // overhang cutout
    }
    #translate([0,outer_diameter/4,0])cube([catch_width, outer_diameter/2+overlap, thickness],center=true); // join sense unit holder to rin
    }
    cylinder(h=thickness, r=inner_diameter/2, center=true); // Ring hole
    
    // hole maker
    rotate([0,0,hole_angle])translate([0,-outer_diameter/2, 0])cube([inner_diameter*0.7,outer_diameter,thickness], center=true);
    }
    
}

// Thumb
ring(thumb_dia, hole_angle=-30);
// Index
translate([-index_dia*1.5,0,0])
ring(index_dia);

// Middle
translate([middle_dia*1.5,0,0])
ring(middle_dia);

// Ring
translate([-middle_dia,middle_dia*1.5,0])
ring(ring_dia);

// Pinky
translate([middle_dia,middle_dia*1.5,0])
ring(pinky_dia);