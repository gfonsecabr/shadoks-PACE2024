// https://github.com/t3nsor/codebook/blob/master/segtree.cpp
#pragma once
#include <vector>

// To enable lazy propagation and range updates, uncomment the following line.
// #define LAZY
template <class Number>
struct Segtree {
  int n;
  std::vector<Number> data;

  Segtree(Number* begin, Number* end): n(end - begin) {
    build_rec(0, begin, end);
  }

  // Call this to update a value (indices are 0-based). If lazy propagation is enabled, use update_range(pos, pos+1, val) instaed.
  void update(int pos, int val) {
    update_rec(0, 0, n, pos, val);
  }

  // Returns sum in range [begin, end). Indices are 0-based.
  Number query(int begin, int end) {
    return query_rec(0, 0, n, begin, end);
  }

protected:
  void build_rec(int node, Number* begin, Number* end) {
    if (end == begin+1) {
      if (data.size() <= node) data.resize(node+1);
      data[node] = *begin;
    } else {
      Number* mid = begin + (end-begin+1)/2;
      build_rec(2*node+1, begin, mid);
      build_rec(2*node+2, mid, end);
      data[node] = data[2*node+1] + data[2*node+2];
    }
  }
  void update_rec(int node, int begin, int end, int pos, Number val) {
    if (end == begin+1) {
      data[node] = val;
    } else {
      int mid = begin + (end-begin+1)/2;
      if (pos < mid) {
        update_rec(2*node+1, begin, mid, pos, val);
      } else {
        update_rec(2*node+2, mid, end, pos, val);
      }
      data[node] = data[2*node+1] + data[2*node+2];
    }
  }
  Number query_rec(int node, int tbegin, int tend, int abegin, int aend) {
    if (tbegin >= abegin && tend <= aend) {
      return data[node];
    } else {
      int mid = tbegin + (tend - tbegin + 1)/2;
      Number res = 0;
      if (mid > abegin && tbegin < aend)
        res += query_rec(2*node+1, tbegin, mid, abegin, aend);
      if (tend > abegin && mid < aend)
        res += query_rec(2*node+2, mid, tend, abegin, aend);
      return res;
    }
  }
};

