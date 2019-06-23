use crate::ai::Position;

use std::collections::BinaryHeap;
use std::cmp::{max, Reverse};

const DX: [isize; 4] = [0, 1, 0, -1];
const DY: [isize; 4] = [1, 0, -1, 0];
const DCHRS: [char; 4] = ['A', 'W', 'D', 'S'];
const LIMIT: isize = 1000;

pub fn shortest(board: &Vec<Vec<char>>, start: &Position, end: &Position)
    -> Vec<char>
{

    let max_x = max(start.0, end.0) as usize + 1;
    let max_y = max(start.1, end.1) as usize + 1;

    let height = board.len();
    let width = board[0].len();

    let inf: isize = max_x as isize * max_y as isize + 10;
    let mut memo = vec![vec![inf; width]; height];
    memo[start.0 as usize][start.1 as usize] = 0;

    let mut heap = BinaryHeap::new();
    heap.push((Reverse(0), (start.0 as isize, start.1 as isize)));

    while let Some((Reverse(dist), (i, j))) = heap.pop() {
        if dist > LIMIT { continue }
        if i == end.0 as isize && j == end.1 as isize {
            break;
        }
        for k in 0..4 {
            let next = (i + DX[k], j + DY[k]);
            if next.0 < 0 || next.0 >= height as isize ||
                next.1 < 0 || next.1 >= width as isize {
                continue
            }
            if board[next.0 as usize][next.1 as usize] == '#' { continue }
            if memo[next.0 as usize][next.1 as usize] > dist + 1 {
                memo[next.0 as usize][next.1 as usize] = dist + 1;
                heap.push((Reverse(dist + 1), next));
            }
        }
    }

    let mut route: Vec<char> = vec![];

    // failed
    if memo[end.0 as usize][end.1 as usize] == inf {
        return route;
    }

    {
        let mut u = end.clone();
        while u != *start {
            for k in 0..4 {
                let next = Position(u.0 + DX[k], u.1 + DY[k]);
                if next.0 < 0 || next.0 >= height as isize ||
                    next.1 < 0 || next.1 >= width as isize {
                    continue
                }
                if board[next.0 as usize][next.1 as usize] == '#' { continue }
                if memo[u.0 as usize][u.1 as usize] == memo[next.0 as usize][next.1 as usize] + 1 {
                    route.push(DCHRS[k]);
                    u = next;
                    break;
                }
            }
        }
    }

    route.reverse();
    route
}

pub fn paint(board: &Vec<Vec<char>>, filled: &Vec<Vec<bool>>,
             start_x: isize, start_y: isize,
             min_x: isize, min_y: isize,
             max_x: isize, max_y: isize,
             ) -> Option<Vec<char>> {

    let height = board.len();
    let width = board[0].len();

    let mut cands = vec![];
    for x in min_x..=max_x {
        for y in min_y..=max_y {
            if x < 0 || x >= height as isize || y < 0 || y >= width as isize { continue }
            if board[x as usize][y as usize] != '#' && !filled[x as usize][y as usize] {
                cands.push((x, y));
            }
        }
    }
    if cands.len() == 0 {
        return None;
    }

    let inf: isize = max_x as isize * max_y as isize + 10;
    let mut memo = vec![vec![inf; width]; height];
    memo[start_x as usize][start_y as usize] = 0;

    let mut heap = BinaryHeap::new();
    heap.push((Reverse(0), (start_x as isize, start_y as isize)));

    while let Some((Reverse(dist), (i, j))) = heap.pop() {
        if dist > LIMIT { continue }
        // if min_x <= i && i <= max_x && min_y <= j && j <= max_y { break; }  // goal
        for k in 0..4 {
            let next = (i + DX[k], j + DY[k]);
            if next.0 < 0 || next.0 >= height as isize ||
                next.1 < 0 || next.1 >= width as isize {
                continue
            }
            if board[next.0 as usize][next.1 as usize] == '#' { continue }
            if memo[next.0 as usize][next.1 as usize] > dist + 1 {
                memo[next.0 as usize][next.1 as usize] = dist + 1;
                heap.push((Reverse(dist + 1), next));
            }
        }
    }

    let (_, end) = cands.iter().map(|p| (memo[p.0 as usize][p.1 as usize], p)).min().unwrap();

    let mut route: Vec<char> = vec![];

    // failed
    if memo[end.0 as usize][end.1 as usize] == inf {
        return None;
    }

    {
        let mut u = end.clone();
        while (u.0, u.1) != (start_x, start_y) {
            for k in 0..4 {
                let next = (u.0 + DX[k], u.1 + DY[k]);
                if next.0 < 0 || next.0 >= height as isize ||
                    next.1 < 0 || next.1 >= width as isize {
                    continue
                }
                if board[next.0 as usize][next.1 as usize] == '#' { continue }
                if memo[u.0 as usize][u.1 as usize] == memo[next.0 as usize][next.1 as usize] + 1 {
                    route.push(DCHRS[k]);
                    u = next;
                    break;
                }
            }
        }
    }

    route.reverse();
    Some(route)
}
