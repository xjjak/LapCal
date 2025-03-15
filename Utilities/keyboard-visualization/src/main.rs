use std::io::{self, BufRead};
use crossterm::{
    execute, terminal,
    cursor,
    // cursor::{Hide, Show}
};



const LINE: &str = "+---+ +---+ +---+ +---+ +---+       +---+ +---+ +---+ +---+ +---+";
const THUMB_LINE: &str = "                  +---+ +---+       +---+ +---+                  ";

fn column_from_byte(byte: &u8) -> [bool; 3] {
    // let mut output = [false; 3];
    // output[0] = (byte >> 0) & 1 != 0;
    // output[1] = (byte >> 1) & 1 != 0;
    // output[2] = (byte >> 2) & 1 != 0;
    [(byte >> 0) & 1 != 0, (byte >> 1) & 1 != 0, (byte >> 2) & 1 != 0]
}

struct Hand {
    thumb: [bool; 3],
    index: [bool; 3],
    index2: [bool; 3],
    middle: [bool; 3],
    ring: [bool; 3],
    pinky: [bool; 3]
}

impl Hand {
    fn finger_by_column(&self, col: u32) -> &[bool; 3] {
	match col {
	    0 => &self.pinky,
	    1 => &self.ring,
	    2 => &self.middle,
	    3 => &self.index,
	    4 => &self.index2,
	    _ => panic!()
	}
    }
    fn update(&mut self, data: &[u8]) {
	self.pinky = column_from_byte(&data[0]);
	self.ring = column_from_byte(&data[1]);
	self.middle = column_from_byte(&data[2]);
	self.index = column_from_byte(&(data[3] & 7)); // Pass only the first 3 bits
	self.index2 = column_from_byte(&(data[3] >> 3)); // Pass next three bits
	self.thumb = column_from_byte(&data[4]);
    }
    fn empty_finger() -> [bool; 3] {
	[false; 3]
    }
    fn new_hand() -> Self {
	Hand {
	    thumb: Hand::empty_finger(),
	    index: Hand::empty_finger(),
	    index2: Hand::empty_finger(),
	    middle: Hand::empty_finger(),
	    ring: Hand::empty_finger(),
	    pinky: Hand::empty_finger()
	}
    }
}

struct KeyboardState {
    left: Hand,
    right: Hand
}

impl KeyboardState {
     fn print(&self) {
	let mut keys_buf = String::new();
	for row in 0..3 {
	    keys_buf.clear();
	    println!("{}", LINE);
	    for column in 0..5 {
		if self.left.finger_by_column(column)[row] {
		    // keys_buf.push_str("| \x1b[42mX\x1b[0m |"); // green background color
		    keys_buf.push_str("| X |");
		} else {
		    keys_buf.push_str("|   |");
		}
		keys_buf.push_str(" ");
	    }
	    keys_buf.push_str("      ");
	    for column in (0..5).rev() {
		if self.right.finger_by_column(column)[row] {
		    keys_buf.push_str("| X |");
		} else {
		    keys_buf.push_str("|   |");
		}
		keys_buf.push_str(" ");
	    }
	    keys_buf.pop();
	    println!("{}", keys_buf);
	    println!("{}", LINE);
	}
	println!("{}", THUMB_LINE);
	keys_buf.clear();
	println!("                  | {} | | {} |       | {} | | {} |                  ",
		 match self.left.thumb[0] { true => "X", false => " "},
		 match self.left.thumb[1] { true => "X", false => " "},
		 match self.right.thumb[1] { true => "X", false => " "},
		 match self.right.thumb[0] { true => "X", false => " "});
	println!("{}", THUMB_LINE);
     }
    fn update(&mut self, data: &[u8]) {
	self.left.update(&data[0..5]);
	self.right.update(&data[5..11]);
    }
    fn new() -> Self {
	KeyboardState {
	    left: Hand::new_hand(),
	    right: Hand::new_hand()
	}
    }
}

fn main() {
    let mut current_state = KeyboardState::new();
    let mut input_buf = vec![];
    let stdin = io::stdin();
    let mut stdout = io::stdout();
    let mut update = true;
    
    // ctrlc::set_handler(move || {execute!(io::stdout(), Show).expect("Show cursor."); run = false;});    
    // execute!(stdout, Hide).expect("Hide cursor"); // Disabled for now, requires signal (exit) handling to re-show
    
    loop {

	if update {
	    current_state.print();
	    update = false;
	}

	// Read byte sequence from stdin
	let mut handle = stdin.lock();
	match handle.read_until(b'\n', &mut input_buf).expect("No problem reading from stdin") {
	    // Parse input & update KeyboardState
	    11 => {current_state.update(&input_buf); update = true;},
	    6 => {current_state.left.update(&input_buf[..5]); update = true;},
	    _ => continue
	}
	

	// Clear terminal (last state)
	execute!(stdout, cursor::MoveToPreviousLine(3*4), terminal::Clear(terminal::ClearType::FromCursorDown)).expect("No problem clearing last output");
	
	input_buf.clear();
    }
}
