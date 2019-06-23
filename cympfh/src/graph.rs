use crate::ai::{AI, Position, Direction, Command};

use std::collections::BinaryHeap;
use std::cmp::{max, Reverse};

const LIMIT: isize = 1000;

pub fn shortest(board: &Vec<Vec<char>>, start: &Position, end: &Position) -> Vec<Direction>
{
    let max_x = max(start.0, end.0) as usize + 1;
    let max_y = max(start.1, end.1) as usize + 1;

    let height = board.len();
    let width = board[0].len();

    let inf: isize = max_x as isize * max_y as isize + 10;
    let mut memo = vec![vec![inf; width]; height];
    let mut prev = vec![vec![(0, 0); width]; height];
    memo[start.0 as usize][start.1 as usize] = 0;

    let mut heap = BinaryHeap::new();
    heap.push((Reverse(0), (start.0 as isize, start.1 as isize)));

    while let Some((Reverse(dist), (i, j))) = heap.pop() {
        if dist > LIMIT { continue }
        if i == end.0 as isize && j == end.1 as isize {
            break;
        }
        for &d in [Direction::Left, Direction::Right, Direction::Up, Direction::Down].iter() {
            let v = d.to_pos();
            let next = (i + v.0, j + v.1);
            if next.0 < 0 || next.0 >= height as isize ||
                next.1 < 0 || next.1 >= width as isize {
                continue
            }
            let next_x = next.0 as usize;
            let next_y = next.1 as usize;
            if board[next_x][next_y] == '#' { continue }
            if memo[next_x][next_y] > dist + 1 {
                memo[next_x][next_y] = dist + 1;
                heap.push((Reverse(dist + 1), next));
                prev[next_x][next_y] = (i, j);
            }
        }
    }

    let mut route: Vec<Direction> = vec![];

    // failed
    if memo[end.0 as usize][end.1 as usize] == inf {
        return route;
    }

    {
        let mut u = (end.0, end.1);
        while u != (start.0, start.1) {
            let v = prev[u.0 as usize][u.1 as usize];
            route.push(match (u.0 - v.0, u.1 - v.1) {
                (1, 0) => Direction::Down,
                (0, 1) => Direction::Right,
                (0, -1) => Direction::Left,
                _ => Direction::Up,
            });
            u = v;
        }
    }

    route.reverse();
    route
}

pub fn paint(ai: &AI,
             worker_idx: usize,
             start_x: isize, start_y: isize,
             min_x: isize, min_y: isize,
             max_x: isize, max_y: isize,
             ) -> Option<(Position, Vec<Command>)>
{
    let height = ai.board.len();
    let width = ai.board[0].len();

    let mut cands: Vec<Position> = vec![];
    for x in min_x..=max_x {
        for y in min_y..=max_y {
            if x < 0 || x >= height as isize || y < 0 || y >= width as isize { continue }
            if ai.board[x as usize][y as usize] != '#' && !ai.filled[x as usize][y as usize] {
                cands.push(Position(x, y));
            }
        }
    }
    if cands.len() == 0 {
        return None;
    }

    // rotate?
    {
        for &cw in [true, false].iter() {
            for &p in ai.rotated_manipulator_positions(worker_idx, cw).iter() {
                for &q in cands.iter() {
                    if p == q {
                        return Some((q, vec![Command::Rotate(cw)]));
                    }
                }
            }
        }
    }

    // move more
    let inf: isize = max_x as isize * max_y as isize + 10;
    let mut memo = vec![vec![inf; width]; height];
    let mut prev = vec![vec![(0, 0); width]; height];
    let mut heap = BinaryHeap::new();

    memo[start_x as usize][start_y as usize] = 0;
    heap.push((Reverse(0), (start_x, start_y)));

    while let Some((Reverse(cost), (i, j))) = heap.pop() {

        if cost > LIMIT { continue }
        // if min_x <= i && i <= max_x && min_y <= j && j <= max_y { break; }  // goal

        for &d in [Direction::Left, Direction::Right, Direction::Up, Direction::Down].iter() {

            let v = d.to_pos();
            let next = (i + v.0, j + v.1);
            if next.0 < 0 || next.0 >= height as isize ||
                next.1 < 0 || next.1 >= width as isize {
                continue
            }
            let next_x = next.0 as usize;
            let next_y = next.1 as usize;
            if ai.board[next_x][next_y] == '#' { continue }

            let cost_next = cost + if ai.filled[next_x][next_y] {
                2
            } else {
                1
            };

            if memo[next_x][next_y] > cost_next {
                memo[next_x][next_y] = cost_next;
                // println!("{} {} ; {}", next_x, next_y, cost_next);
                heap.push((Reverse(cost_next), next));
                prev[next_x][next_y] = (i, j);
            }
        }
    }

    let (_, end) = cands.iter().map(|p| (memo[p.0 as usize][p.1 as usize], p)).min_by_key(|&(cost, _)| cost).unwrap();

    // fail
    if memo[end.0 as usize][end.1 as usize] == inf {
        return None;
    }

    let mut route: Vec<Command> = vec![];

    {
        let mut u = (end.0, end.1);
        while u != (start_x, start_y) {
            let v = prev[u.0 as usize][u.1 as usize];
            route.push(match (u.0 - v.0, u.1 - v.1) {
                (1, 0) => Command::Move(Direction::Down),
                (0, 1) => Command::Move(Direction::Right),
                (0, -1) => Command::Move(Direction::Left),
                _ => Command::Move(Direction::Up),
            });
            u = v;
        }
    }

    route.reverse();
    Some((*end, route))
}
