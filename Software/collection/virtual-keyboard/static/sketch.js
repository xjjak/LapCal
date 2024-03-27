// constants
const SCALER_WIDTH = 100;

// default sizes & keyboard measurements
let w = 734, h = 667;
let aspectratio = w/h;
let key_size = [125/w, 125/h]
let kb_positions = [
    [[0/w, 125/h],
     [0/w, 260/h],
     [0/w, 395/h],
     [1-key_size[0]-135/w, 1-key_size[1]-35/h]],
    [[135/w, 42/h],
     [135/w, 177/h],
     [135/w, 312/h],
     [1-key_size[0], 1-key_size[1]]],
    [[270/w, 0/h],
     [270/w, 135/h],
     [270/w, 270/h]],
    [[405/w, 38/h],
     [405/w, 173/h],
     [405/w, 308/h]],
    [[540/w, 56/h],
     [540/w, 191/h],
     [540/w, 326/h]],
];

// colors per finger
let finger_coloring = [];

// Button positions
let saveButton = [0,0,100/w,50/h];
let flipButton = [0,50/h,100/w,50/h];


// global variables
let logs = []; // the actual log
let suggested_key = [0,0];
let touchPositions = new Map();

// flags
let mirror = false; // mirror the shown keyboard?
let bottom_aligned = true;
let send_to_server = true;


// auxiliary functions
const onButtonClick = (button, func, ...args) => (tx, ty) => {
    let [sx,sy,sw,sh] = button;
    if (mirror)
        sx = 1-(sx+sw);
    if (sx*w <= tx && tx <= (sx+sw)*w &&
        sy*h <= ty && ty <= (sy+sh)*h) {
        func(...args);
        return true;
    } else {
        return false;
    }
};


// Initialisation
function setup() {
    createCanvas(windowWidth, windowHeight);
    
    // set colors per finger
    finger_coloring = [
        color("#cdb4db"),
        color("#ffc8dd"),
        color("#ffafcc"),
        color("#bde0fe"),
        color("#81a1c1")
    ];

    // Select initial random key for suggestion
    new_suggested_key();
}

function draw() {
    // fill background
    background(255);

    // highlight scaler area
    push()
    fill(150);
    noStroke();
    rect(windowWidth-SCALER_WIDTH, 0, SCALER_WIDTH, windowHeight);
    pop();

    // draw all keyboard keys
    kb_positions.forEach((finger, i) => {
        finger.forEach((pos, j) => {
            // relative bounding box considering `mirror`
            let [x, y] = pos;
            if (mirror) x = 1 - (x+key_size[0]);

            push();
            fill(finger_coloring[i]);
            // add border when the key is suggested
            if (suggested_key[0] == i && suggested_key[1] == j) {
                strokeWeight(4);
                stroke(255,0,0);
            }

            // draw rect
            rect(w*x, bottom_aligned*(windowHeight-h)+h*y, w*key_size[0], h*key_size[1]);
            pop();
        })
    });

    // mark all currently pressed keys
    push();
    fill(0, 255, 0);
    touchPositions.forEach((idx, id) => {
        let [x, y] = kb_positions[idx[0]][idx[1]];
        if (mirror) x = 1 - (x+key_size[0]);
        rect(w*x, bottom_aligned*(windowHeight-h)+h*y, w*key_size[0], h*key_size[1]);
    });
    pop();
    
    // draw save button
    fill(255,230,230);
    let [sx,sy,sw,sh] = saveButton;
    if (mirror)
        sx = 1-(sx+sw);
    rect(sx*w,sy*h,sw*w,sh*h);

    // draw flip button
    fill(230,230,255);
    [sx,sy,sw,sh] = flipButton;
    if (mirror)
        sx = 1-(sx+sw);
    rect(sx*w,sy*h,sw*w,sh*h);

    // fix hanging press
    if (touches.length == 0)
        touchPositions = new Map();
}

// process initiated touch events
function touchStarted(event) {
    let tx = event.changedTouches[0].clientX;
    let ty = event.changedTouches[0].clientY;

    // check for extra buttons
    if (onButtonClick(saveButton, saveToFile)(tx, ty))
        return false;
    if (onButtonClick(flipButton, flip)(tx, ty))
        return false;
    
    idx = kb_positions.map((finger, fi) => {
        return finger.map((pos, pi) => [pos, [fi, pi]]);
    }).flat().reduce((acc, el) => {
        let [[x, y], new_idx] = el;
        x += key_size[0]/2;
        y += key_size[1]/2;
        if (mirror) x = 1 - (x+key_size[0]);

        let new_dist = (tx - x*w)**2 + ((ty - bottom_aligned*(windowHeight-h)) - y*h)**2;
        let [dist, idx] = acc;
        if (new_dist < dist)
            return [new_dist, new_idx];
        else
            return acc;
    }, [Infinity, [0, 0]])[1];


    log_now(["1", idx[0].toString(), idx[1].toString()], sep=",");
    // re-generate suggested_key if the last one was pressed
    if (suggested_key[0] == idx[0] && suggested_key[1] == idx[1])
        new_suggested_key();
    
    touchPositions.set(event.changedTouches[0].identifier, idx);
    
    return false;
}

// adjust size on mouse dragging on scaler
function mouseDragged() {
    if (mouseX > windowWidth - SCALER_WIDTH) {
        if (bottom_aligned) {
            if (mouseY > windowHeight - h/2)
                bottom_aligned = false;
            w = aspectratio*(windowHeight-mouseY);
            h = windowHeight-mouseY;
        } else {
            if (mouseY < h/2)
                bottom_aligned = true;
            w = aspectratio*mouseY;
            h = mouseY;
        }
    }
}

// process finalised touch events
function touchEnded(event) {
    if (!touchPositions.has(event.changedTouches[0].identifier))
        return false;
    
    let idx = touchPositions.get(event.changedTouches[0].identifier);
    touchPositions.delete(event.changedTouches[0].identifier);

    log_now(["0", idx[0].toString(), idx[1].toString()], sep=",");
    
    return false;
}

function log_now(msgs, sep = " ") {
    log_str = (1000*Date.now()).toString() + sep + msgs.join(sep);
    logs.push(log_str);

    // Send new message to server
    fetch("http://" + IPADDR + ":8080", {
        method: "POST",
        body: log_str,
        headers: {
            "Content-type": "text/plain; charset=UTF-8"
        }
    });
}

// save log to txt file and clear log
function saveToFile() {
    saveStrings(logs, Date.now().toString()+".txt");
    logs = [];
}

// change mirror state and clear log
function flip() {
    mirror = !mirror;
    logs = [];
}

function new_suggested_key() {
    suggested_key[0] = int(random(kb_positions.length));
    suggested_key[1] = int(random(kb_positions[suggested_key[0]].length));
}
