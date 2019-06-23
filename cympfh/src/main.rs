#[allow(unused_imports)]
use std::env;
use std::fs::File;
use std::io::prelude::*;
use std::io::{BufReader, BufRead};
use std::cmp::{max, min};

mod ai;
mod geo;
mod graph;

use ai::{AI, Direction, Position, Command};
use graph::{shortest_path, udon_shortest_path, paint, udon_paint};

extern crate ncurses;
use ncurses::*;

enum EditorMode {
    Normal,
    CursorExtension,
    CursorShortestPath,
    CursorRectPaintFirst,
    CursorRectPaintSecond(Position),
    CursorUdonShortestPath,
    CursorUdonRectPaintFirst,
    CursorUdonRectPaintSecond(Position),
}

const WINDOW_HEIGHT: usize = 40;
// const WINDOW_WIDTH: usize = 100;

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

    let window_height_range = if ai.height <= WINDOW_HEIGHT {
        0..ai.height
    } else {
        let left = if ai.workers[0].current_pos.0 as usize > WINDOW_HEIGHT / 2 {
            ai.workers[0].current_pos.0 as usize - WINDOW_HEIGHT / 2
        } else {
            0
        };
        left..min(left + WINDOW_HEIGHT, ai.height)
    };
    let i_offset = window_height_range.start as i32;

    for i in window_height_range {
        for j in 0..ai.width {

            wmove(win, i as i32 + 4 - i_offset, j as i32);

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
    wmove(win, WINDOW_HEIGHT as i32 + 4, 0);
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
    const CHAR_L: i32 = 'l' as i32;
    const CHAR_O: i32 = 'o' as i32;
    const CHAR_Q: i32 = 'q' as i32;
    const CHAR_S: i32 = 's' as i32;
    const CHAR_U: i32 = 'u' as i32;
    const CHAR_V: i32 = 'v' as i32;
    const CHAR_W: i32 = 'w' as i32;
    const CHAR_X: i32 = 'x' as i32;
    const CHAR_LARGE_X: i32 = 88;
    const CHAR_LARGE_V: i32 = 86;
    const CHAR_RET: i32 = 10;
    const CHAR_UP: i32 = 65;
    const CHAR_DOWN: i32 = 66;
    const CHAR_RIGHT: i32 = 67;
    const CHAR_LEFT: i32 = 68;
    const CHAR_HELP: i32 = '?' as i32;
    const CHAR_RESET: i32 = '<' as i32;

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
                    message = String::from("Using B: Choose a cell (move cursor and hit Enter)");
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
            // hyper methods
            CHAR_X => {
                message = format!("Shortest Path Mode: Choose a cell");
                mode = EditorMode::CursorShortestPath;
                cursor = ai.workers[0].current_pos;
                changed = false;
            },
            CHAR_LARGE_X => {
                message = format!("Udon Shortest Path Mode: Choose a cell");
                mode = EditorMode::CursorUdonShortestPath;
                cursor = ai.workers[0].current_pos;
                changed = false;
            },
            CHAR_V => {
                message = format!("Rect Paint: Choose one of corner");
                mode = EditorMode::CursorRectPaintFirst;
                cursor = ai.workers[0].current_pos;
                changed = false;
            },
            CHAR_LARGE_V => {
                message = format!("Udon Rect Paint: Choose one of corner");
                mode = EditorMode::CursorUdonRectPaintFirst;
                cursor = ai.workers[0].current_pos;
                changed = false;
            },
            // cursor move
            CHAR_UP => {
                cursor.0 -= 1;
                message = format!("Cursor at {:?}", cursor);
                changed = false;
            },
            CHAR_RIGHT => {
                cursor.1 += 1;
                message = format!("Cursor at {:?}", cursor);
                changed = false;
            },
            CHAR_DOWN => {
                cursor.0 += 1;
                message = format!("Cursor at {:?}", cursor);
                changed = false;
            },
            CHAR_LEFT => {
                cursor.1 -= 1;
                message = format!("Cursor at {:?}", cursor);
                changed = false;
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
                    EditorMode::CursorShortestPath => {
                        if let Some(commands) = shortest_path(&ai, 0, cursor) {
                            for &cmd in commands.iter() {
                                match cmd {
                                    Command::Move(d) => {
                                        ai.mv(0, d);
                                    },
                                    Command::Rotate(cw) => {
                                        if cw {
                                            ai.turn_cw(0);
                                        } else {
                                            ai.turn_ccw(0);
                                        }
                                    },
                                    _ => {}
                                }
                            }
                            message = format!("Shortest path to {:?}", &cursor);
                            changed = true;
                        } else {
                            message = format!("!!Not found any Shortest path to {:?}", &cursor);
                            changed = false;
                        }
                        cursor = Position(-1, -1);
                        mode = EditorMode::Normal;
                    },
                    EditorMode::CursorUdonShortestPath => {
                        if let Some(commands) = udon_shortest_path(&ai, 0, cursor) {
                            for &cmd in commands.iter() {
                                match cmd {
                                    Command::Move(d) => {
                                        ai.mv(0, d);
                                    },
                                    Command::Rotate(cw) => {
                                        if cw {
                                            ai.turn_cw(0);
                                        } else {
                                            ai.turn_ccw(0);
                                        }
                                    },
                                    _ => {}
                                }
                            }
                            message = format!("Udon Shortest path to {:?}", &cursor);
                            changed = true;
                        } else {
                            message = format!("Not found any Udon Shortest path to {:?}", &cursor);
                            changed = false;
                        }
                        cursor = Position(-1, -1);
                        mode = EditorMode::Normal;
                    },
                    EditorMode::CursorRectPaintFirst => {
                        message = format!("Rect Paint: You choose {:?} as a corner. Choose another corner", cursor);
                        mode = EditorMode::CursorRectPaintSecond(cursor.clone());
                        changed = false;
                    },
                    EditorMode::CursorRectPaintSecond(p) => {
                        message = format!("Rect Painting: {:?} to {:?}", &p, &cursor);
                        let min_x = min(p.0, cursor.0);
                        let max_x = max(p.0, cursor.0);
                        let min_y = min(p.1, cursor.1);
                        let max_y = max(p.1, cursor.1);

                        loop {
                            let Position(wx, wy) = ai.workers[0].current_pos;
                            if let Some((q, commands)) = paint(&ai, 0, wx, wy, min_x, min_y, max_x, max_y) {
                                for cmd in commands {
                                    if ai.filled[q.0 as usize][q.1 as usize] { break }
                                    let success = match cmd {
                                        Command::Move(dir) => {
                                            ai.mv(0, dir)
                                        },
                                        Command::Rotate(cw) => {
                                            if cw {
                                                ai.turn_cw(0)
                                            } else {
                                                ai.turn_ccw(0)
                                            }
                                        },
                                        _ => {
                                            false
                                        }
                                    };
                                    if !success { break }
                                }
                            } else {
                                break;
                            }
                        }

                        changed = true;
                        cursor = Position(-1, -1);
                        mode = EditorMode::Normal;
                    },
                    EditorMode::CursorUdonRectPaintFirst => {
                        message = format!("Udon Rect Paint: You choose {:?} as a corner. Choose another corner", cursor);
                        mode = EditorMode::CursorUdonRectPaintSecond(cursor.clone());
                        changed = false;
                    },
                    EditorMode::CursorUdonRectPaintSecond(p) => {
                        message = format!("Udon Rect Painting: {:?} to {:?}", &p, &cursor);

                        let rect_min = Position(min(p.0, cursor.0), min(p.1, cursor.1));
                        let rect_max = Position(max(p.0, cursor.0), max(p.1, cursor.1));

                        loop {
                            if let Some((q, commands)) = udon_paint(&ai, 0, rect_min, rect_max) {
                                for cmd in commands {
                                    if ai.filled[q.0 as usize][q.1 as usize] { break }
                                    let success = match cmd {
                                        Command::Move(dir) => {
                                            ai.mv(0, dir)
                                        },
                                        Command::Rotate(cw) => {
                                            if cw {
                                                ai.turn_cw(0)
                                            } else {
                                                ai.turn_ccw(0)
                                            }
                                        },
                                        _ => {
                                            false
                                        }
                                    };
                                    if !success { break }
                                }
                            } else {
                                break;
                            }
                        }

                        changed = true;
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
            CHAR_O => {
                message = format!("Save output: {}.handout", args[1]);
                let mut file = File::create(format!("{}.hand.out", args[1])).unwrap();
                let _ = file.write_all(ai.print_commands().as_bytes());
            },
            CHAR_RESET => {
                message = format!("New Game");
                ai = history[0].clone();
                history = vec![ai.clone()];
                changed = false;
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

    {
        let mut file = File::create(format!("{}.hand.out", args[1])).unwrap();
        let _ = file.write_all(ai.print_commands().as_bytes());
    }

}
