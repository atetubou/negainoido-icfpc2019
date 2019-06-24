use crate::ai::{AI, Position, Direction, Command};

use std::collections::BinaryHeap;
use std::cmp::Reverse;

const LIMIT: isize = 1000;

pub fn shortest_path(ai: &AI, idx: usize, dst: Position) -> Option<Vec<Command>> {

    if dst.0 < 0 || dst.0 >= ai.height as isize || dst.1 < 0 || dst.1 >= ai.width as isize {
        return None;
    }
    if ai.board[dst.0 as usize][dst.1 as usize] == '#' {
        return None;
    }
    if ai.filled[dst.0 as usize][dst.1 as usize] {
        return None;
    }

    let inf: isize = (ai.height * ai.width) as isize + 10;
    println!("inf is {}", inf);

    let mut memo = vec![vec![inf; ai.width]; ai.height];
    let mut prev = vec![vec![Position(0, 0); ai.width]; ai.height];

    let Position(wx, wy) = ai.workers[idx].current_pos;
    memo[wx as usize][wy as usize] = 0;

    let mut heap = BinaryHeap::new();
    heap.push((Reverse(0), ai.workers[idx].current_pos));

    while let Some((Reverse(dist), p)) = heap.pop() {

        if dist > LIMIT { continue }
        if p == dst { break }

        for &d in [Direction::Left, Direction::Right, Direction::Up, Direction::Down].iter() {
            let v = d.to_pos();
            let next = Position(p.0 + v.0, p.1 + v.1);
            if next.0 < 0 || next.0 >= ai.height as isize ||
                next.1 < 0 || next.1 >= ai.width as isize  {
                continue
            }
            let next_x = next.0 as usize;
            let next_y = next.1 as usize;
            if ai.board[next_x][next_y] == '#' { continue }
            if memo[next_x][next_y] > dist + 1 {
                memo[next_x][next_y] = dist + 1;
                heap.push((Reverse(dist + 1), next));
                prev[next_x][next_y] = p;
            }
        }
    }

    // failed
    if memo[dst.0 as usize][dst.1 as usize] == inf {
        return None;
    }

    let mut commands: Vec<Command> = vec![];
    {
        let mut u = dst;
        while u != ai.workers[idx].current_pos {
            let v = prev[u.0 as usize][u.1 as usize];
            let d = match (u.0 - v.0, u.1 - v.1) {
                (1, 0) => Direction::Down,
                (0, 1) => Direction::Right,
                (0, -1) => Direction::Left,
                _ => Direction::Up,
            };
            commands.push(Command::Move(d));
            u = v;
        }
    }
    commands.reverse();
    Some(commands)
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

pub fn udon_shortest_path(ai: &AI, idx: usize, dst: Position) -> Option<Vec<Command>> {

    if dst.0 < 0 || dst.0 >= ai.height as isize || dst.1 < 0 || dst.1 >= ai.width as isize {
        return None;
    }
    if ai.board[dst.0 as usize][dst.1 as usize] == '#' {
        return None;
    }
    if ai.filled[dst.0 as usize][dst.1 as usize] {
        return None;
    }

    let mut world = ai.clone();

    use std::collections::VecDeque;
    use std::collections::HashSet;
    let mut q = VecDeque::new();
    let mut used = HashSet::new();

    q.push_back((world.workers[idx].clone(), vec![]));

    while let Some((worker, history)) = q.pop_front() {

        let w = worker.clone();
        if used.contains(&w) { continue }
        used.insert(w);

        for &cw in [true, false].iter() {
            world.workers[idx] = worker.clone();
            let cmd = Command::Rotate(cw);
            let mut history_next = history.clone();
            let success = if cw {
                world.turn_cw(idx)
            } else {
                world.turn_ccw(idx)
            };
            if !success { continue }
            history_next.push(cmd);
            if world.filled[dst.0 as usize][dst.1 as usize] {
                return Some(history_next);
            }
            q.push_back((world.workers[idx].clone(), history_next));
        }
        for &d in [Direction::Left, Direction::Right, Direction::Up, Direction::Down].iter() {
            world.workers[idx] = worker.clone();
            let cmd = Command::Move(d);
            let mut history_next = history.clone();
            let success = world.mv(idx, d);
            if !success { continue }
            history_next.push(cmd);
            if world.filled[dst.0 as usize][dst.1 as usize] {
                return Some(history_next);
            }
            q.push_back((world.workers[idx].clone(), history_next));
        }
    };

    None
}

pub fn udon_paint(ai: &AI, idx: usize, rect_min: Position, rect_max: Position) -> Option<(Position, Vec<Command>)> {

    let mut length = ai.height * ai.width + 10;
    let mut ret = None;

    for x in rect_min.0..=rect_max.0 {
        for y in rect_min.1..=rect_max.1 {
            if let Some(routes) = udon_shortest_path(&ai, idx, Position(x, y)) {
                if routes.len() < length {
                    length = routes.len();
                    ret = Some((Position(x, y), routes));
                }
            }
        }
    }
    ret
}
