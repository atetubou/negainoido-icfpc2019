#pragma once

#include <complex>
#include <vector>

typedef std::complex<double> Point;

struct Line : public std::vector<Point> {
  Line(const Point &a, const Point &b) {
    push_back(a); push_back(b);
  }
};

bool intersectLL(const Line &l, const Line &m);
bool intersectLS(const Line &l, const Line &s);
bool intersectLP(const Line &l, const Point &p);

bool intersectSS(const Line &s, const Line &t);

bool intersectSP(const Line &s, const Point &p);
