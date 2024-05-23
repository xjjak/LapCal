use <Round-Anything/polyround.scad>


overlap = 0.001;
thumb_dia = 16.0;
index_dia = 14.5;
middle_dia = 15.5;
ring_dia = 14.0;
pinky_dia = 12.0;
thumb_hole_width = 75;
thumb_hole_angle = -30;


$fa = 1;
$fs = 0.4;

module ring(inner_diameter, tolerance=1.5, hole_width_angle=75, hole_angle=0){
    tol = tolerance;
    strength = 1.5;
    inner_diameter = inner_diameter + tolerance;
    outer_diameter = inner_diameter + strength*2;
    thickness = 2;
    pcb_width = 13;
    pcb_height = 1;
    catch_wall = 1.5;
    catch_width = pcb_width + 2*catch_wall;
    catch_overhang = 0.5;
    prong_height = 2;
    catch_height = pcb_height + prong_height + 0.01;
    corner_rad = 0.5;
    
    difference(){

    union(){
    cylinder(h = thickness, r=outer_diameter/2, center=true); // Outer ring

    // difference(){
        translate([0, outer_diameter/2, -thickness/2])linear_extrude(thickness){
        polygon(polyRound([
            [-catch_width/2, 0, 0],// bottom`left`
            [-outer_diameter/2, -outer_diameter/2, 0],
            [outer_diameter/2, -outer_diameter/2, 0],
            [catch_width/2, 0 , 0],// bottom right
            [catch_width/2, catch_height, corner_rad],
            [catch_width/2-catch_wall-catch_overhang, catch_height, corner_rad],
            [catch_width/2-catch_wall-catch_overhang, catch_height-prong_height, 0.5],
            [catch_width/2-catch_wall, catch_height-prong_height, 0],
            [catch_width/2-catch_wall, catch_height-prong_height-pcb_height, 0],
            [-catch_width/2+catch_wall, catch_height-prong_height-pcb_height, 0],
            [-catch_width/2+catch_wall, catch_height-prong_height, 0],
            [-catch_width/2+catch_wall+catch_overhang, catch_height-prong_height, 0.5],
            [-catch_width/2+catch_wall+catch_overhang, catch_height, corner_rad],
            [-catch_width/2, catch_height, corner_rad]
            ], 30));
        }
    }
    
    
    cylinder(h=thickness, r=inner_diameter/2, center=true); // Ring hole
    
    // hole maker

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
