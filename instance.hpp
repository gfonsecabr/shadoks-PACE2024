#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>

using i64 = long long int;

auto beginTime = std::chrono::high_resolution_clock::now();
std::mt19937 rgen(1);

inline double elapsed() {
  auto end = std::chrono::high_resolution_clock::now();
  auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - beginTime);

  return dur.count() / 1000.0;
}

inline std::vector<std::string> splitString(const std::string &s, char sep = ' ') {
  std::vector<std::string> v;
  std::string cur = "";

  for(char c : s) {
    if(c == sep) {
      if(cur.size() != 0)
        v.push_back(cur);
      cur = "";
    }
    else
      cur += c;
  }

  if(cur.size() != 0)
    v.push_back(cur);

  return v;
}

struct Instance {
  int n0 = 0, n1 = 0, m = 0;
  std::vector<std::vector<int>> adj;
  std::vector<int> v1;
  int v1Degree = 0;

  Instance() {}

  Instance(std::string fn) {
    std::ifstream f(fn);

    std::string line;
    int curm = 0;
    while (!f ? std::getline(std::cin, line) : std::getline(f, line)) {
      if(line.empty() || line[0] == 'c')
        continue; // Comment line
      if(line[0] == 'p') {
        auto v = splitString(line);
        assert(v.size() >= 5); // 5 for exact and heuristic, 6 for parameterized
        n0 = std::stoi(v[2]);
        n1 = std::stoi(v[3]);
        m = std::stoi(v[4]);
        adj.resize(n0+n1);
      }
      else {
        auto v = splitString(line);
        if(v.size() == 2) {
          int a,b;
          a = std::stoi(v[0]) - 1;
          b = std::stoi(v[1]) - 1;
          adj.at(a).push_back(b);
          adj.at(b).push_back(a);
          curm++;
        }
      }
    }

    for(int i = 0; i < n0+n1; i++) {
      std::sort(adj[i].begin(), adj[i].end());
    }

    for(int i = n0; i < n0+n1; i++) {
      if(adj[i].size())
        v1.push_back(i-n0);
      v1Degree = std::max(v1Degree, (int)adj[i].size());
    }
  }

  ~Instance() {
  }
};


