#[derive(Debug, Clone)]
pub struct Point(pub f64, pub f64);

#[derive(Debug, Clone)]
pub struct Line(pub Point, pub Point);

const EPS: f64 = 1e-8;

fn dot(a: &Point, b: &Point) -> f64 {
  a.0 * b.0 + a.1 * b.1
}

fn cross(a: &Point, b: &Point) -> f64 {
  a.0 * b.1 - a.1 * b.0
}

fn norm(a: &Point) -> f64 {
  (a.0 * a.0 + a.1 * a.1).sqrt()
}

fn ccw_2(b: &Point, c: &Point) -> i32 {
  if cross(&b, &c) > 0.0 {
    1
  } else if cross(&b, &c) < 0.0 {
    -1
  } else if dot(&b, &c) < 0.0 {
    2
  } else if norm(&b) < norm(&c) {
    -2
  } else {
    0
  }
}

fn ccw(a: &Point, b: &Point, c: &Point) -> i32 {
  ccw_2(&Point(b.0 - a.0, b.1 - a.1),
        &Point(c.0 - a.0, c.1 - a.1))
}

pub fn intersectSS(s: &Line, t: &Line) -> bool {
  ccw(&s.0, &s.1,&t.0)*ccw(&s.0,&s.1,&t.1) < 0 && ccw(&t.0,&t.1,&s.0)*ccw(&t.0,&t.1,&s.1) < 0
}

fn dist(p: &Point, q: &Point) -> f64 {
  ((p.0 - q.0) * (p.0 - q.0) + (p.1 - q.1) * (p.1 - q.1)).sqrt()
}

pub fn intersectSP(s: &Line, p: &Point) -> bool {
  (dist(&s.0, p) + dist(&s.1, p) - dist(&s.0, &s.1)).abs() < EPS
}
