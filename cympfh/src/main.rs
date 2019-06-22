#[allow(unused_imports)]
use std::env;
use std::fs::File;
use std::io::{BufReader, BufRead};

mod ai;
mod geo;

use ai::{AI, Direction};

extern crate ncurses;
use ncurses::*;

fn dump(ai: &AI, w: &*mut i8) {

    clear();

    for i in 0..ai.height {
        for j in 0..ai.width {
            wmove(*w, i as i32 + 2, j as i32);
            if ai.workers[0].current_pos.0 == i as isize && ai.workers[0].current_pos.1 == j as isize {
                attrset(COLOR_PAIR(3));

                let me = match ai.workers[0].current_dir {
                    Direction::Left => '<',
                    Direction::Right => '>',
                    Direction::Up => '^',
                    Direction::Down => 'v',
                };
                
                waddch(*w, me as u32);
            } else if ai.filled[i][j] {
                attrset(COLOR_PAIR(2));
                waddch(*w, ai.board[i][j] as u32);
            } else {
                attrset(COLOR_PAIR(1));
                if ai.board[i][j] == 'W' {
                    waddch(*w, '.' as u32);
                } else {
                    waddch(*w, ai.board[i][j] as u32);
                }
            }
        }
    }

    wmove(*w, ai.height as i32 + 4, 0);
    waddstr(*w, &ai.print_commands());
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
    init_pair(3, COLOR_WHITE, COLOR_BLACK);   // self

    const CHAR_A: i32 = 'a' as i32;
    const CHAR_S: i32 = 's' as i32;
    const CHAR_D: i32 = 'd' as i32;
    const CHAR_W: i32 = 'w' as i32;
    const CHAR_Q: i32 = 'q' as i32;
    const CHAR_E: i32 = 'e' as i32;
    const CHAR_H: i32 = 'h' as i32;
    const CHAR_J: i32 = 'j' as i32;
    const CHAR_K: i32 = 'k' as i32;
    const CHAR_L: i32 = 'l' as i32;
    const CHAR_U: i32 = 'u' as i32;
    const CHAR_QUIT: i32 = '<' as i32;

    dump(&ai, &win);
    let mut history = vec![ai.clone()];

    loop {

        wmove(win, 0, 0);
        let mut changed = true;

        match getch() {
            CHAR_A | CHAR_H => {
                ai.mv(0, Direction::Left);
                waddstr(win, "Left");
            },
            CHAR_D | CHAR_L => {
                ai.mv(0, Direction::Right);
                waddstr(win, "Right");
            },
            CHAR_S | CHAR_J => {
                ai.mv(0, Direction::Down);
                waddstr(win, "Down");
            },
            CHAR_W | CHAR_K => {
                ai.mv(0, Direction::Up);
                waddstr(win, "Up");
            },
            CHAR_E => {
                ai.turn_cw(0);
                waddstr(win, "Turn CW");
            },
            CHAR_Q => {
                ai.turn_ccw(0);
                waddstr(win, "Turn CCW");
            },
            CHAR_U => {
                if history.len() == 1 {
                    ai = history[0].clone();
                } else {
                    let _ = history.pop();
                    ai = history[history.len() - 1].clone();
                }
                waddstr(win, &format!("Undo (history={})", history.len()));
                changed = false;
            },
            CHAR_QUIT => {
                break;
            },
            _ => {
                waddstr(win, "Unknown??");
                changed = false;
            }
        }

        if changed {
            history.push(ai.clone());
        }

        dump(&ai, &win);

        if ai.is_finished() {
            break;
        }

    }

    endwin();
    println!("{}", ai.print_commands());
}
