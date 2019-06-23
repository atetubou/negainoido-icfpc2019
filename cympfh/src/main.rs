#[allow(unused_imports)]
use std::env;
use std::fs::File;
use std::io::{BufReader, BufRead};
use std::cmp::{max, min};

mod ai;
mod geo;

use ai::{AI, Direction, Position};

extern crate ncurses;
use ncurses::*;

enum EditorMode {
    Normal,
    CursorExtension,
}

fn dump(ai: &AI, &win: &*mut i8, message: &String, &cursor: &Position) {

    clear();
    attrset(COLOR_PAIR(1));
    wmove(win, 0, 0);
    addstr(message);

    // count
    wmove(win, 1, 0);
    attrset(COLOR_PAIR(1));
    waddstr(win, &format!("count: #B = {}, #F = {}, #L = {}, #R = {}, #C = {}; com-len = {}",
                          ai.count_extension,
                          ai.count_fast,
                          ai.count_drill,
                          0,
                          ai.count_clone,
                          ai.executed_cmds.len(),
                          ));
    wmove(win, 2, 0);
    waddstr(win, &format!("duration: F: {}, L: {}",
                          ai.workers[0].duration_fast,
                          ai.workers[0].duration_drill,
                          ));

    let mybody = ai.get_absolute_manipulator_positions(0);
    let is_my_body = |p: &Position| { mybody.iter().any(|&q| *p == q) };

    for i in 0..ai.height {
        for j in 0..ai.width {

            wmove(win, i as i32 + 4, j as i32);

            let color = if Position(i as isize, j as isize) == cursor {
                5
            } else if ai.filled[i][j] && is_my_body(&Position(i as isize, j as isize)) {
                3
            } else if ai.filled[i][j] {
                2
            } else if ai.board[i][j] == '#' {
                4
            } else {
                1
            };

            let character = if Position(i as isize, j as isize) == ai.workers[0].current_pos {
                match ai.workers[0].current_dir {
                    Direction::Left => '<',
                    Direction::Right => '>',
                    Direction::Up => '^',
                    Direction::Down => 'v',
                }
            } else if ai.filled[i][j] && is_my_body(&Position(i as isize, j as isize)) {
                ai.board[i][j]
            } else if ai.board[i][j] == ' ' || ai.board[i][j] == 'W' {
                ' '
            } else {
                ai.board[i][j]
            };

            attrset(COLOR_PAIR(color));
            waddch(win, character as u32);
        }
    }

    // your command
    wmove(win, ai.height as i32 + 5, 0);
    attrset(COLOR_PAIR(1));
    waddstr(win, &ai.print_commands());
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
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);  // occupied, filled
    init_pair(3, COLOR_BLUE, COLOR_RED);   // self
    init_pair(4, COLOR_BLACK, COLOR_BLACK);   // obs
    init_pair(5, COLOR_BLACK, COLOR_GREEN);   // cursor

    const CHAR_A: i32 = 'a' as i32;
    const CHAR_B: i32 = 'b' as i32;
    const CHAR_D: i32 = 'd' as i32;
    const CHAR_E: i32 = 'e' as i32;
    const CHAR_F: i32 = 'f' as i32;
    // const CHAR_H: i32 = 'h' as i32;
    // const CHAR_J: i32 = 'j' as i32;
    // const CHAR_K: i32 = 'k' as i32;
    const CHAR_L: i32 = 'l' as i32;
    const CHAR_Q: i32 = 'q' as i32;
    const CHAR_S: i32 = 's' as i32;
    const CHAR_U: i32 = 'u' as i32;
    const CHAR_W: i32 = 'w' as i32;
    const CHAR_RET: i32 = 10;
    const CHAR_UP: i32 = 65;
    const CHAR_DOWN: i32 = 66;
    const CHAR_RIGHT: i32 = 67;
    const CHAR_LEFT: i32 = 68;
    const CHAR_HELP: i32 = '?' as i32;
    const CHAR_QUIT: i32 = '<' as i32;

    clear();
    let mut history = vec![ai.clone()];
    let mut cursor = Position(-1, -1);
    let mut mode = EditorMode::Normal;
    dump(&ai, &win, &String::new(), &cursor);

    loop {

        let mut changed = true;
        let message;

        match getch() {
            // worker move
            CHAR_A => {
                if ai.mv(0, Direction::Left) {
                    message = String::from("Left");
                } else {
                    message = String::from("Cannot Left");
                    changed = false;
                }
            },
            CHAR_D => {
                if ai.mv(0, Direction::Right) {
                    message = String::from("Right");
                } else {
                    message = String::from("Cannot Right");
                    changed = false;
                }
            },
            CHAR_S => {
                if ai.mv(0, Direction::Down) {
                    message = String::from("Down");
                } else {
                    message = String::from("Cannot Down");
                    changed = false;
                }
            },
            CHAR_W => {
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
            CHAR_Q => {
                ai.turn_ccw(0);
                message = String::from("Turn CCW");
            },
            // boosters
            CHAR_B => {
                if ai.count_extension == 0 {
                    message = String::from("You have no B");
                    changed = false;
                } else {
                    mode = EditorMode::CursorExtension;
                    changed = false;
                    message = String::from("Choose a cell (move cursor and hit Enter)");
                    cursor = ai.workers[0].current_pos;
                }
            },
            CHAR_L => {
                if ai.use_drill(0) {
                    message = format!("Using Drill (L)");
                } else {
                    message = format!("Cannot Use Drill (L)");
                }
            },
            CHAR_F => {
                if ai.use_fast_wheel(0) {
                    message = format!("Using FastWheel (F)");
                } else {
                    message = format!("Cannot Use FastWheel (F)");
                }
            },
            // cursor move
            CHAR_UP => {
                cursor.0 = max(0, cursor.0 - 1);
                message = format!("Cursor at {:?}", cursor);
            },
            CHAR_RIGHT => {
                cursor.1 = min(ai.width as isize - 1, cursor.1 + 1);
                message = format!("Cursor at {:?}", cursor);
            },
            CHAR_DOWN => {
                cursor.0 = min(ai.height as isize - 1, cursor.0 + 1);
                message = format!("Cursor at {:?}", cursor);
            },
            CHAR_LEFT => {
                cursor.1 = max(0, cursor.1 - 1);
                message = format!("Cursor at {:?}", cursor);
            },
            CHAR_RET => {
                match mode {
                    EditorMode::CursorExtension => {
                        let Position(wx, wy) = ai.workers[0].current_pos;
                        let p = Position(cursor.0 - wx, cursor.1 - wy);
                        if ai.use_extension(0, p) {
                            message = format!("Using Extension (B) at {:?}", p);
                        } else {
                            message = format!("Cannot Apply Extension (B) at {:?}", p);
                            changed = false;
                        }
                        cursor = Position(-1, -1);
                        mode = EditorMode::Normal;
                    },
                    EditorMode::Normal => {
                        message = format!("");
                        changed = false;
                    }
                }
            },
            // editor commands
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
            unknown => {
                message = format!("Unknown keycode: {}", unknown);
                changed = false;
            }
        }

        if changed {
            history.push(ai.clone());
        }

        dump(&ai, &win, &message, &cursor);

        if ai.is_finished() {
            break;
        }

    }

    endwin();
    println!("{}", ai.print_commands());
}
