overlap = 0.001;
thumb_dia = 16;
index_dia = 14;
middle_dia = 15;
ring_dia = 14;
pinky_dia = 12;
thumb_hole_width = 75;
thumb_hole_angle = -30;


$fa = 1;
$fs = 0.4;

// TODO:
// - fillets

module ring(inner_diameter, tolerance=0.2, hole_width_angle=75, hole_angle=0){
    tol = tolerance;
    strength = 1.5;
    outer_diameter = inner_diameter + strength*2;
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
    //rotate([0,0,hole_angle])translate([0,-outer_diameter/2, 0])cube([inner_diameter*0.7,outer_diameter,thickness], center=true);

    #rotate([0,0,hole_angle])translate([0,0,-thickness/2])linear_extrude(height=thickness){
        r = outer_diameter/2;
        hole_x = tan(hole_width_angle/2)*r;
        hole_y = r;
        
        
            
        fillet_rad = strength/2;
        rf = r-fillet_rad;
        fillet_x = rf*sin(hole_width_angle/2);
        fillet_y = -rf*cos(hole_width_angle/2);
        difference(){
        polygon([[0,0],[-hole_x, -hole_y], [hole_x, -hole_y]]);
        translate([fillet_x, fillet_y, 0])circle(fillet_rad);
        translate([-fillet_x, fillet_y, 0])circle(fillet_rad);
        }
    }

    }
}

// Thumb
ring(thumb_dia, hole_width_angle=thumb_hole_width, hole_angle=thumb_hole_angle);

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