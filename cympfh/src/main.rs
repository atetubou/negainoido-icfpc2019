#[allow(unused_imports)]
use std::env;
use std::fs::File;
use std::io::{BufReader, BufRead};

use rand::prelude::*;

mod ai;
mod geo;

use ai::{AI, Direction, Position};

extern crate ncurses;
use ncurses::*;

fn dump(ai: &AI, w: &*mut i8, message: &String) {

    clear();
    attrset(COLOR_PAIR(1));
    wmove(*w, 0, 0);
    addstr(message);

    // count
    wmove(*w, 2, 0);
    attrset(COLOR_PAIR(1));
    waddstr(*w, &format!("#B = {}, #F = {}, #L = {}, #R = {}, #C = {}",
                         ai.count_extension,
                         ai.count_fast,
                         ai.count_drill,
                         0,
                         ai.count_clone));

    let mybody = ai.get_absolute_manipulator_positions(0);
    let is_my_body = |p: &Position| { mybody.iter().any(|&q| *p == q) };

    for i in 0..ai.height {
        for j in 0..ai.width {
            wmove(*w, i as i32 + 4, j as i32);
            if ai.workers[0].current_pos.0 == i as isize && ai.workers[0].current_pos.1 == j as isize {
                attrset(COLOR_PAIR(3));
                let me = match ai.workers[0].current_dir {
                    Direction::Left => '<',
                    Direction::Right => '>',
                    Direction::Up => '^',
                    Direction::Down => 'v',
                };
                waddch(*w, me as u32);
            } else if ai.filled[i][j] && is_my_body(&Position(i as isize, j as isize)) {
                attrset(COLOR_PAIR(3));
                waddch(*w, '.' as u32);
            } else if ai.filled[i][j] {
                attrset(COLOR_PAIR(2));
                waddch(*w, ai.board[i][j] as u32);
            } else {
                match ai.board[i][j] {
                    '#' => {
                        attrset(COLOR_PAIR(4));
                        waddch(*w, ' ' as u32);
                    },
                    '.' | 'W' => {
                        attrset(COLOR_PAIR(5));
                        waddch(*w, ' ' as u32);
                    },
                    _ => {
                        attrset(COLOR_PAIR(1));
                        waddch(*w, ai.board[i][j] as u32);
                    }
                }
            }
        }
    }

    // your command
    wmove(*w, ai.height as i32 + 5, 0);
    attrset(COLOR_PAIR(1));
    waddstr(*w, &ai.print_commands());
}

fn extension_positions(ai: &AI, idx: usize) -> Vec<Position> {
    let Position(x, y) = ai.workers[idx].current_pos;
    let mut ret = vec![];
    for i in x-10..=x+10 {
        for j in y-10..=y+10 {
            let mut can_use = false;
            for &Position(x, y) in ai.workers[idx].manipulator_range.iter() {
                let dist = (i - x).abs() + (j - y).abs();
                if dist == 0 {
                    can_use = false;
                    break;
                } else if dist == 1 {
                    can_use = true;
                }
            }
            if can_use {
                ret.push(Position(i, j))
            }
        }
    }
    ret
}

fn main() {

    let args: Vec<String> = env::args().collect();

    let mut ai: AI =
    {
        let f = File::open(args[1].as_str()).unwrap();
        let file = BufReader::new(&f);
        let mut first = true;
        let mut h: usize = 0;
        let mut w: usize = 0;
        let mut board: Vec<Vec<char>> = vec![];
        for line in file.lines() {
            if first {

                let size: Vec<usize> = line.ok().unwrap().split_whitespace().map(|x|
                                                 String::from(x).parse().unwrap()).collect();
                h = size[0];
                w = size[1];
                first = false;
            } else {
                let cs: Vec<char> = line.ok().unwrap().chars().collect();
                board.push(cs);
            }
        }
        AI::new(h, w, board)
    };

    for &p in ai.get_absolute_manipulator_positions(0).iter() {
        ai.fill_cell(0, &p);
    }

    let win = initscr();

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);   // default
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);  // occupied
    init_pair(3, COLOR_BLUE, COLOR_RED);   // self
    init_pair(4, COLOR_BLACK, COLOR_BLACK);   // obs
    init_pair(5, COLOR_WHITE, COLOR_WHITE);   // empty

    const CHAR_A: i32 = 'a' as i32;
    const CHAR_B: i32 = 'b' as i32;
    const CHAR_D: i32 = 'd' as i32;
    const CHAR_E: i32 = 'e' as i32;
    const CHAR_H: i32 = 'h' as i32;
    const CHAR_J: i32 = 'j' as i32;
    const CHAR_K: i32 = 'k' as i32;
    const CHAR_L: i32 = 'l' as i32;
    const CHAR_Q: i32 = 'q' as i32;
    const CHAR_S: i32 = 's' as i32;
    const CHAR_U: i32 = 'u' as i32;
    const CHAR_W: i32 = 'w' as i32;
    const CHAR_HELP: i32 = '?' as i32;
    const CHAR_QUIT: i32 = '<' as i32;

    clear();
    dump(&ai, &win, &String::new());
    let mut history = vec![ai.clone()];

    loop {

        let mut changed = true;
        let mut message = String::new();

        match getch() {
            CHAR_A | CHAR_H => {
                if ai.mv(0, Direction::Left) {
                    message = String::from("Left");
                } else {
                    message = String::from("Cannot Left");
                    changed = false;
                }
            },
            CHAR_D | CHAR_L => {
                if ai.mv(0, Direction::Right) {
                    message = String::from("Right");
                } else {
                    message = String::from("Cannot Right");
                    changed = false;
                }
            },
            CHAR_S | CHAR_J => {
                if ai.mv(0, Direction::Down) {
                    message = String::from("Down");
                } else {
                    message = String::from("Cannot Down");
                    changed = false;
                }
            },
            CHAR_W | CHAR_K => {
                if ai.mv(0, Direction::Up) {
                    message = String::from("Up");
                } else {
                    message = String::from("Cannot Up");
                    changed = false;
                }
            },
            CHAR_E => {
                ai.turn_cw(0);
                message = String::from("Turn CW");
            },
            CHAR_B => {
                let ps = ai.extension_positions(0);
                if ps.len() == 0 {
                    changed = false;
                    message = String::from("No Candidates for Extension (B)");
                } else {
                    let i: usize = rand::random();
                    if ai.use_extension(0, ps[i % ps.len()]) {
                        message = format!("Using Extension (B) at {:?}", ps[i % ps.len()]);
                    } else {
                        message = format!("Cannot Apply Extension (B) at {:?}", ps[i % ps.len()]);
                    }
                }
            },
            CHAR_Q => {
                ai.turn_ccw(0);
                message = String::from("Turn CCW");
            },
            CHAR_U => {
                if history.len() == 1 {
                    ai = history[0].clone();
                } else {
                    let _ = history.pop();
                    ai = history[history.len() - 1].clone();
                }
                message = format!("Undo (history={})", history.len());
                changed = false;
            },
            CHAR_QUIT => {
                break;
            },
            CHAR_HELP => {
                message = String::from("Move: a/s/d/w, Rotate: q/e, Boost: b/l/f/c");
            }
            _ => {
                message = String::from("Unknown??");
                changed = false;
            }
        }

        if changed {
            history.push(ai.clone());
        }

        dump(&ai, &win, &message);

        if ai.is_finished() {
            break;
        }

    }

    endwin();
    println!("{}", ai.print_commands());
}
