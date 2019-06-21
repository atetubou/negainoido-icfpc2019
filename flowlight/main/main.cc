#include <ctime>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include <glog/logging.h>
using namespace std;

#define REP2(i, m, n) for(int i = (int)(m); i < (int)(n); i++)
#define REPD(i, n) for(int i = (int)(n) - 1; i >= 0; i--)
#define REP(i, n) REP2(i, 0, n)
#define ALL(c) (c).begin(), (c).end()
#define PB(e) push_back(e)
#define FOREACH(i, c) for(auto i = (c).begin(); i != (c).end(); ++i)
#define MP(a, b) make_pair(a, b)
#define BIT(n, m) (((n) >> (m)) & 1)

typedef long long ll;

template <typename S, typename T> ostream &operator<<(ostream &out, const pair<S, T> &p) {
  out << "(" << p.first << ", " << p.second << ")";
  return out;
}

template <typename T> ostream &operator<<(ostream &out, const vector<T> &v) {
  out << "[";
  REP(i, v.size()){
    if (i > 0) out << ", ";
    out << v[i];
  }
  out << "]";
  return out;
}

const int MAX_H = 1000;
const int MAX_W = 1000;
const int MAX_V = MAX_H * MAX_W;

typedef pair<int, int> Position;

int toIndex(const Position &position) {
  return position.first * MAX_W + position.second;
}

int toIndex(int h, int w) {
  return h * MAX_W + w;
}

Position toPosition(const int index) {
  return Position(index / MAX_W, index % MAX_W);
}

class UnionFind{
  int num_components;
  std::vector<int> parent;
  std::vector<int> weight;
  std::vector<int> rank;
public:
  UnionFind(int N) : num_components(N),
                     parent(std::vector<int>(N)),
                     weight(std::vector<int>(N, 1)),
                     rank(std::vector<int>(N, 0)){
    for(int i = 0; i < N; i++) parent[i] = i;
  }
  
  int find(int x){
    if(x == parent[x]) return x;
    else return parent[x] = find(parent[x]);
  }
  
  int size(int x){
    return weight[find(x)];
  }

  
  bool same(int x, int y){
    return find(x) == find(y);
  }
    
  void unite(int x, int y){
    x = find(x);
    y = find(y);
    if(x == y) return;

    num_components--;
    if(rank[x] < rank[y]){
      weight[y] += weight[x];
      parent[x] = y;
    }else{
      weight[x] += weight[y];
      parent[y] = x;
      if(rank[x] == rank[y]) rank[y]++;
    }
  }
  
  int count(){
    return num_components;
  }
};

struct Edge {
  int srcIndex;
  int dstIndex;
  int weight;
  Edge(const int srcIndex, const int dstIndex, int weight) :
    srcIndex(srcIndex), dstIndex(dstIndex), weight(weight) { }

  Edge reverse() const {
    return Edge(dstIndex, srcIndex, weight);
  }

  friend ostream &operator<<(ostream &out, const Edge &e) {
    out << "Edge(" << e.srcIndex << toPosition(e.srcIndex)  << ", " << e.dstIndex << toPosition(e.dstIndex) << ")";
    return out;
  }
};

vector<Edge> computeTraversal(const int v, const vector<vector<Edge>> &G, vector<int> &visited) {
  visited[v] = 1;
  vector<Edge> es;
  for (const auto &e: G[v]) {
    if (!visited[e.dstIndex]) {
      es.push_back(e);
      vector<Edge> sub_es = computeTraversal(e.dstIndex, G, visited);
      for (const auto &sub_e: sub_es) {
        es.push_back(sub_e);
      }
      es.push_back(e.reverse());
    }
  }
  return es;
}

vector<Edge> computeTraversal(const int startIndex, const vector<vector<Edge>> &G) {
  vector<int> visited(G.size());
  return computeTraversal(startIndex, G, visited);
}

string getMove(const Edge &e) {
  const int dh = toPosition(e.dstIndex).first - toPosition(e.srcIndex).first;
  const int dw = toPosition(e.dstIndex).second - toPosition(e.srcIndex).second;
  if (dh == 1) return "W";
  if (dh == -1) return "S";
  if (dw == 1) return "D";
  if (dw == -1) return "A";
  return "";
}

int dh[] = {-1, 0, 1, 0};
int dw[] = {0, -1, 0, 1};


int main(int argc, char** argv) {
  int H, W;
  cin >> H >> W;
  vector<string> board(H);

  
  REP(i, H) cin >> board[H - 1 - i];
  cerr << board << endl;
  vector<Edge> edges;
  UnionFind uf(MAX_H * MAX_W);

  // find start index
  int startIndex = -1;
  REP(h, H) REP(w, W) {
    if (board[h][w] == 'W') {
      startIndex = toIndex(h, w);
    }
  }
  LOG_IF(FATAL, startIndex < 0);

  // construct MST
  vector<int> requiredVertices;
  requiredVertices.push_back(toIndex(0, 0));
  requiredVertices.push_back(toIndex(1, 0));
  requiredVertices.push_back(toIndex(2, 0));
  requiredVertices.push_back(toIndex(0, 3));
  requiredVertices.push_back(toIndex(1, 3));
  requiredVertices.push_back(toIndex(2, 3));
  requiredVertices.push_back(toIndex(1, 6));

  REP(h, H) REP(w, W) {
    if (board[h][w] == '#') continue;
    REP(k, 4) {
      int nh = h + dh[k];
      int nw = w + dw[k];
      if (0 <= nh && nh < H && 0 <= nw && nw < W && board[nh][nw] != '#') {
        edges.push_back(Edge(toIndex(h, w), toIndex(nh, nw), 1));
      }
    }
  }

  sort(ALL(edges), [](const Edge &e1, const Edge &e2) { return e1.weight < e2.weight; } );
  vector<Edge> mstEdges;
  for (const auto &e: edges) {
    if (!uf.same(e.srcIndex, e.dstIndex)) {
      mstEdges.push_back(e);
      uf.unite(e.srcIndex, e.dstIndex);
    }
  }
  
  vector<vector<Edge>> G(MAX_V);
  for (const auto &e: mstEdges) {
    G[e.srcIndex].push_back(e);
    G[e.dstIndex].push_back(e.reverse());
  }

  size_t targetCount = 0;
  REP(h, H) REP(w, W) if (board[h][w] != '#') targetCount++;
  set<int> visitedIndices;
  visitedIndices.insert(startIndex);
  vector<Edge> edgeOrder = computeTraversal(startIndex, G);

  string res = "";
  for (const auto &e: edgeOrder) {
    res += getMove(e);
    visitedIndices.insert(e.dstIndex);
    if (visitedIndices.size() == targetCount) break;
  }
  cout << res << endl;
  return 0;
}
