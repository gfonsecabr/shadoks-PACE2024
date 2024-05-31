#pragma once
#include <utility>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include "instance.hpp"
#include "segtree.hpp"

using Coords = std::vector<int>;
using Order = std::vector<int>;

Coords normalizedC(const Coords &coords, int k = 1) {
  Coords ret(coords.size());

  std::vector<std::pair<int, int>> pairs;
  for(unsigned i = 0; i < coords.size(); i++)
    pairs.push_back(std::make_pair(coords[i], i));
  std::sort(pairs.begin(), pairs.end());
  for(unsigned i = 0; i < pairs.size(); i++) {
    ret[pairs[i].second] = k * i;
  }

  return ret;
}

void normalizeC(Coords &coords, int k = 1) {
  std::vector<std::pair<int, int>> pairs;
  for(unsigned i = 0; i < coords.size(); i++)
    pairs.push_back(std::make_pair(coords[i], i));
  std::sort(pairs.begin(), pairs.end());
  for(unsigned i = 0; i < pairs.size(); i++) {
    coords[pairs[i].second] = k * i;
  }
}

Order toOrder(Coords &coords) {
  Order ret(coords.size());
  std::vector<std::pair<int, int>> pairs;
  for(unsigned i = 0; i < coords.size(); i++)
    pairs.push_back(std::make_pair(coords[i], i));
  std::sort(pairs.begin(), pairs.end());
  for(unsigned i = 0; i < pairs.size(); i++) {
    ret[i] = pairs[i].second;
  }
  return ret;
}

Coords toCoords(Instance &inst, Order &order) {
  Coords ret(inst.n1);
  for(int i = 0; i < order.size(); i++) {
    ret.at(order[i]) = i;
  }
  return ret;
}

template<class STREAM>
void printOrSave(Instance &inst, Order &ord, STREAM &stream) {
  std::unordered_set<int> listed;
  for(const auto &v : ord) {
    stream << inst.n0 + v + 1 << std::endl;
    listed.insert(v);
  }
  for(int v = 0; v < inst.n1; v++) {
    if(!listed.contains(v))
      stream << inst.n0 + v + 1 << std::endl;
  }
}

void save(Instance &inst, Order &ord, std::string fn) {
  std::ofstream f(fn);
  printOrSave(inst, ord, f);
}

void print(Instance &inst, Order &ord) {
  printOrSave(inst, ord, std::cout);
}

Order load(Instance &inst, std::string fn) {
  std::ifstream f(fn);
  Order sol;
  while (true) {
    int x;
    f >> x;
    if( f.eof() ) break;
    sol.push_back(x - inst.n0 - 1);
  }
  return sol;
}

i64 crossingsC(Instance &inst, Coords coords) {
  normalizeC(coords);
  std::vector<int> v(inst.n1,0);
  Segtree<int> tree(v.data(), v.data() + v.size());
  i64 ret = 0;
  for(int top = inst.n0-1; top >= 0; top--) {
    for(int bottom : inst.adj[top]) {
      int y = coords[bottom-inst.n0];
      ret += tree.query(0, y);
    }
    for(int bottom : inst.adj[top]) {
      int y = coords[bottom-inst.n0];
      v.at(y)++;
      tree.update(y,v[y]);
    }
  }
  return ret;
}

i64 crossings(Instance &inst, std::vector<int>::iterator begin, std::vector<int>::iterator end) {
  std::vector<int> v(inst.n0,0);
  Segtree<int> tree(v.data(), v.data() + v.size());
  i64 ret = 0;

  for(auto bottom = end-1; bottom != begin-1; bottom--) {
    for(int top : inst.adj[*bottom+inst.n0]) {
      ret += tree.query(0, top);
    }
    for(int top : inst.adj[*bottom+inst.n0]) {
      v.at(top)++;
      tree.update(top, v[top]);
    }
  }

  return ret;
}

// This version could be faster for small slices, but does not seem to be
//
// i64 crossingsSmall(Instance &inst, std::vector<int>::iterator begin, std::vector<int>::iterator end) {
//   std::map<int, int> relTop;
//   for(auto bottom = begin; bottom != end; bottom++)
//     for(int x : inst.adj[*bottom+inst.n0])
//       relTop[x] = -1;
//
//   {
//     int i = 0;
//     for(auto &[_,val] : relTop) {
//       val = i++;
//     }
//   }
//
//   std::vector<int> v(relTop.size(),0);
//   Segtree<int> tree(v.data(), v.data() + v.size());
//   i64 ret = 0;
//
//   for(auto bottom = end-1; bottom != begin-1; bottom--) {
//     for(int top : inst.adj[*bottom+inst.n0]) {
//       ret += tree.query(0, relTop[top]);
//     }
//     for(int top : inst.adj[*bottom+inst.n0]) {
//       int i = relTop[top];
//       v.at(i)++;
//       tree.update(i, v[i]);
//     }
//   }
//
//   return ret;
// }









