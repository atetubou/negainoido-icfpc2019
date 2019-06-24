#![allow(dead_code)]

use std::cmp::{min, max};
use std::collections::HashSet;

use crate::geo::{Line, Point, intersect_ss, intersect_sp};

#[derive(Debug, Clone, Copy, Hash, PartialEq, Eq)]
pub enum Direction {
  Right,
  Down,
  Left,
  Up,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Position(pub isize, pub isize);

#[derive(Debug, Clone, Copy)]
pub enum Command {
  Move(Direction),
  Rotate(bool),
  Boost
}

#[derive(Debug, Clone, Hash, PartialEq, Eq)]
pub struct Worker {
    pub current_dir: Direction,
    pub current_pos: Position,
    pub manipulator_range: Vec<Position>,
    pub duration_drill: usize,
    pub duration_fast: usize,
}

impl Position {
    fn rotate(&self, dir: Direction) -> Position {
        match dir {
            Direction::Right => self.clone(),
            Direction::Up => Position(-self.1, self.0),
            Direction::Left => Position(-self.0, -self.1),
            Direction::Down => Position(self.1, -self.0),
        }
    }
    fn rotate_reverse(&self, dir: Direction) -> Position {
        self.rotate(dir).rotate(dir).rotate(dir)
    }
}

impl Direction {
    pub fn to_pos(&self) -> Position {
        match self {
            Direction::Right => Position(0, 1),
            Direction::Down => Position(1, 0),
            Direction::Left => Position(0, -1),
            Direction::Up => Position(-1, 0),
        }
    }
    pub fn cw(&self) -> Direction {
        match self {
            Direction::Right => Direction::Down,
            Direction::Down => Direction::Left,
            Direction::Left => Direction::Up,
            Direction::Up => Direction::Right,
        }
    }
    pub fn ccw(&self) -> Direction {
        match self {
            Direction::Right => Direction::Up,
            Direction::Down => Direction::Right,
            Direction::Left => Direction::Down,
            Direction::Up => Direction::Left,
        }
    }
}

impl Worker {
    fn new(p: Position) -> Self {
        Worker {
            current_dir: Direction::Right,
            current_pos: p,
            manipulator_range: vec![
                Position(0, 0),
                Position(1, 1),
                Position(0, 1),
                Position(-1, 1)
            ],
            duration_drill: 0,
            duration_fast: 0,
        }
    }
}

#[derive(Debug, Clone)]
pub struct AI {
    pub current_time: usize,
    pub height: usize,
    pub width: usize,
    pub count_fast: usize,
    pub count_drill: usize,
    pub count_extension: usize,
    pub count_clone: usize,
    pub count_teleport: usize,
    pub filled_count: usize,
    pub block_count: usize,
    pub workers: Vec<Worker>,
    pub beacons: HashSet<Position>,
    pub executed_cmds: Vec<Vec<String>>,
    pub board: Vec<Vec<char>>,
    pub filled: Vec<Vec<bool>>,
}

impl AI {

    pub fn new(h: usize, w: usize, board: Vec<Vec<char>>) -> Self {
        let w_pos = {
            let mut pos = Position(0, 0);
            for i in 0..h {
                for j in 0..w {
                    if board[i][j] == 'W' {
                        pos = Position(i as isize, j as isize);
                    }
                }
            }
            pos
        };
        let block_count = board.iter().map(|row|
                  row.iter().filter(|&c| *c == '#').count()).sum();
        AI {
            current_time: 0,
            height: h,
            width: w,
            count_fast: 0,
            count_drill: 0,
            count_extension: 0,
            count_clone: 0,
            count_teleport: 0,
            filled_count: 0,
            block_count: block_count,
            workers: vec![Worker::new(w_pos)],
            executed_cmds: vec![vec![]],
            beacons: HashSet::new(),
            board: board,
            filled: vec![vec![false; w]; h],
        }
    }

    fn valid_pos(&self, p: &Position) -> bool {
        let Position(x, y) = *p;
        0 <= x && x < self.height as isize && 0 <= y && y < self.width as isize
    }

    pub fn reachable(&self, idx: usize, p: &Position) -> bool {
        if !self.valid_pos(p) { return false; }
        let Position(x, y) = *p;
        if self.board[x as usize][y as usize] == '#' { return false; }
        let Position(wx, wy) = self.workers[idx].current_pos;
        for cx in min(x, wx)..=max(x, wx) {
          for cy in min(y, wy)..=max(y, wy) {
            if self.board[cx as usize][cy as usize] != '#' { continue }
            let l = Line(Point(wx as f64 +0.5, wy as f64 + 0.5),
                         Point(x as f64 + 0.5, y as f64 + 0.5));
            let cell = Point(cx as f64 + 0.5, cy as f64 + 0.5);
            if intersect_sp(&l, &cell) { return false; }

            let corners = vec![
              Point(cx as f64, cy as f64),
              Point(cx as f64 + 1.0, cy as f64),
              Point(cx as f64 + 1.0, cy as f64 + 1.0),
              Point(cx as f64, cy as f64 + 1.0),
            ];
            for i in 0..4 {
              let edge = Line(corners[i].clone(), corners[(i + 1) % 4].clone());
              if intersect_ss(&l, &edge) { return false; }
            }
          }
        }
        true
    }

    pub fn fill_cell(&mut self, idx: usize, p: &Position) -> bool {
        let Position(x, y) = *p;
        if !self.valid_pos(p) { return false; }
        if !self.reachable(idx, p) { return false; }

        // pick up when item is on body,
        if self.workers[idx].current_pos == *p {  // when on body
          match self.board[x as usize][y as usize] {
              'B' => { self.count_extension += 1; },
              'F' => { self.count_fast += 1; },
              'L' => { self.count_drill += 1; },
              'C' => { self.count_clone += 1; },
              'R' => { self.count_teleport += 1; },
              _ => {}
          }
          self.board[x as usize][y as usize] = '.';
        }

        if self.filled[x as usize][y as usize] { return false; }
        self.filled[x as usize][y as usize] = true;
        self.filled_count += 1;
        true
    }

    pub fn get_neighbor(&self, idx: usize, d: Direction) -> Position {
        let Position(x, y) = self.workers[idx].current_pos;
        let Position(dx, dy) = d.to_pos();
        Position(x + dx, y + dy)
    }

    pub fn turn_cw(&mut self, idx: usize) -> bool {
        self.workers[idx].current_dir = match self.workers[idx].current_dir {
            Direction::Right => Direction::Down,
            Direction::Down => Direction::Left,
            Direction::Left => Direction::Up,
            Direction::Up => Direction::Right,
        };
        self.executed_cmds[idx].push(String::from("E"));
        for &p in self.get_absolute_manipulator_positions(idx).iter() {
            self.fill_cell(idx, &p);
        }
        self.next_turn();
        true
    }

    pub fn turn_ccw(&mut self, idx: usize) -> bool {
        self.workers[idx].current_dir = match self.workers[idx].current_dir {
            Direction::Right => Direction::Up,
            Direction::Up => Direction::Left,
            Direction::Left => Direction::Down,
            Direction::Down => Direction::Right,
        };
        self.executed_cmds[idx].push(String::from("Q"));
        for &p in self.get_absolute_manipulator_positions(idx).iter() {
            self.fill_cell(idx, &p);
        }
        self.next_turn();
        true
    }

    pub fn get_absolute_manipulator_positions(&self, idx: usize) -> Vec<Position> {
        let Position(x, y) = self.workers[idx].current_pos;
        let mut ret = vec![];
        for &p in self.workers[idx].manipulator_range.iter() {
            let q = p.rotate(self.workers[idx].current_dir);
            ret.push(Position(x + q.0, y + q.1));
        }
        ret
    }

    pub fn rotated_manipulator_positions(&self, idx: usize, cw: bool) -> Vec<Position> {
        let mut ret = vec![];
        let Position(wx, wy) = self.workers[idx].current_pos;
        let ndir = if cw {
          self.workers[idx].current_dir.cw()
        } else {
          self.workers[idx].current_dir.ccw()
        };
        for &p in self.workers[idx].manipulator_range.iter() {
            let mani = p.rotate(ndir);
            let q = Position(mani.0 + wx, mani.1 + wy);
            if self.reachable(idx, &q) {
                ret.push(q);
            }
        }
        ret
    }

    pub fn moved_manipulator_positions(&self, idx: usize, d: Direction) -> Vec<Position> {
        let mut ret = vec![];
        if !self.try_move(idx, d) { return ret }
        let Position(wx, wy) = self.workers[idx].current_pos;
        let v = d.to_pos();
        for &p in self.workers[idx].manipulator_range.iter() {
            let q = Position(wx + p.0 + v.0, wy + p.1 + v.1);
            if self.reachable(idx, &q) {
                ret.push(q);
            }
        }
        ret
    }

    fn next_turn(&mut self) {
        self.current_time += 1;
        for i in 0..self.workers.len() {
            if self.workers[i].duration_drill > 0 {
                self.workers[i].duration_drill -= 1;
            }
            if self.workers[i].duration_fast > 0 {
                self.workers[i].duration_fast -= 1;
            }
        }
    }

    fn try_move(&self, idx: usize, dir: Direction) -> bool {
        let next_pos = self.get_neighbor(idx, dir);
        if !self.valid_pos(&next_pos) {
            return false;
        }
        let Position(x, y) = next_pos;
        if self.board[x as usize][y as usize] == '#' && self.workers[idx].duration_drill == 0 {
            return false;
        }
        true
    }

    fn move_body(&mut self, idx: usize, dir: Direction) -> bool {
        // validation
        if !self.try_move(idx, dir) { return false; }
        // move
        self.workers[idx].current_pos = self.get_neighbor(idx, dir);

        let Position(wx, wy) = self.workers[idx].current_pos;
        if self.board[wx as usize][wy as usize] == '#' {
          self.board[wx as usize][wy as usize] = '.';
          self.block_count -= 1;
        }

        // fill && pick up
        for &p in self.get_absolute_manipulator_positions(idx).iter() {
            self.fill_cell(idx, &p);
        }
        true
    }

    pub fn mv(&mut self, idx: usize, dir: Direction) -> bool {
        if !self.move_body(idx, dir) { return false; }
        // when FAST
        if self.workers[idx].duration_fast > 0 {
            self.move_body(idx, dir);
        }
        // push command
        self.executed_cmds[idx].push(match dir {
            Direction::Up => String::from("W"),
            Direction::Down => String::from("S"),
            Direction::Left => String::from("A"),
            Direction::Right => String::from("D"),
        });
        self.next_turn();
        true
    }

    pub fn is_finished(&self) -> bool {
        self.filled_count + self.block_count == self.height * self.width
    }

    pub fn print_commands(&self) -> String {
        self.executed_cmds.iter().map(|cmds| cmds.join("")).collect::<Vec<String>>().join("#")
    }

    pub fn use_fast_wheel(&mut self, idx: usize) -> bool {
        if self.count_fast == 0 {
            return false;
        }
        self.workers[idx].duration_fast = 50;
        self.count_fast -= 1;
        self.executed_cmds[idx].push(String::from("F"));
        true
    }

    pub fn use_drill(&mut self, idx: usize) -> bool {
        if self.count_drill == 0 {
            return false;
        }
        self.workers[idx].duration_drill = 30;
        self.count_drill -= 1;
        self.executed_cmds[idx].push(String::from("L"));
        true
    }

    pub fn use_clone(&mut self, idx: usize) -> bool {
        if self.count_clone == 0 {
            return false;
        }

        let Position(wx,wy) = self.workers[idx].current_pos;
        if self.board[wx as usize][wy as usize] != 'X' {
            return false;
        }

        self.count_clone -= 1;

        self.workers.push(Worker::new(Position(wx, wy)));

        self.executed_cmds.push(vec![]);
        self.executed_cmds[idx].push(String::from("C"));
        true
    }

    pub fn install_beacon(&mut self, idx: usize) -> bool {
        if self.count_teleport == 0 {
            return false;
        }
        let Position(wx, wy) = self.workers[idx].current_pos;
        if self.board[wx as usize][wy as usize] == 'X' || self.board[wx as usize][wy as usize] == 'b' {
            return false;
        }

        self.count_teleport -= 1;
        self.board[wx as usize][wy as usize] = 'b';

        self.beacons.insert(Position(wx,wy));

        self.executed_cmds[idx].push(String::from("R"));

        true
    }

    pub fn jump_to_beacon(&mut self, idx: usize, dst: &Position) -> bool {
        if !self.beacons.contains(dst) {
            return false;
        }

        self.workers[idx].current_pos.0 = dst.0;
        self.workers[idx].current_pos.1 = dst.1;

        self.executed_cmds[idx].push(format!("T({},{})",dst.0, dst.1));
        true
    }

    pub fn nop(&mut self, idx: usize) -> bool {
        self.executed_cmds[idx].push(String::from("Z"));
        true
    }

    // return relative positions
    pub fn extension_positions(&self, idx: usize) -> Vec<Position> {
        let Position(wx, wy) = self.workers[idx].current_pos;
        let mybody = self.get_absolute_manipulator_positions(0);
        let is_my_body = |p: &Position| { mybody.iter().any(|&q| *p == q) };
        let mut ret = vec![];
        for &Position(x, y) in mybody.iter() {
            for p in vec![Position(x + 1, y), Position(x - 1, y), Position(x, y + 1), Position(x, y - 1)] {
                if !is_my_body(&p) {
                    ret.push(Position(p.0 - wx, p.1 - wy));
                }
            }
        }
        ret
    }

    // p is relative
    pub fn use_extension(&mut self, idx: usize, p: Position) -> bool {
        if self.count_extension == 0 {
            return false;
        }
        if !self.extension_positions(idx).iter().any(|&q| q == p) {
          return false;
        }
        self.count_extension -= 1;
        self.executed_cmds[idx].push(format!("B({},{})", p.1, -p.0));
        {
          let q = p.rotate_reverse(self.workers[idx].current_dir);
          self.workers[idx].manipulator_range.push(q);
        }
        for &p in self.get_absolute_manipulator_positions(idx).iter() {
            self.fill_cell(idx, &p);
        }
        true
    }
}
