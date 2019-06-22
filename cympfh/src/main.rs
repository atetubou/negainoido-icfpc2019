#[allow(unused_imports)]
use rand::prelude::*;

mod ai;
mod geo;

use ai::AI;

fn main() {
    let mut sc = Scanner::new();
    let h: usize = sc.cin();
    let w: usize = sc.cin();
    let board: Vec<Vec<char>> = (0..h).map(|_| sc.cin::<String>().chars().collect()).collect();
    let mut ai = AI::new(h, w, board);
}

use std::io::{self, Write};
use std::str::FromStr;
use std::collections::VecDeque;

struct Scanner { stdin: io::Stdin, buffer: VecDeque<String>, }
impl Scanner {
    fn new() -> Self { Scanner { stdin: io::stdin(), buffer: VecDeque::new() } }
    fn cin<T: FromStr>(&mut self) -> T {
        while self.buffer.len() == 0 {
            let mut line = String::new();
            let _ = self.stdin.read_line(&mut line);
            for w in line.split_whitespace() {
                self.buffer.push_back(String::from(w));
            }
        }
        self.buffer.pop_front().unwrap().parse::<T>().ok().unwrap()
    }
}
