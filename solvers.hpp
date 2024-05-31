#pragma once

#include "instance.hpp"
#include "solution.hpp"

template<class DT, class DTM>
class Solvers {
  DTM *matrix = nullptr;
  int maxcoord = 0;
  size_t msize = 0;

public:
  Solvers(){}

  Solvers(size_t memlimit) {
    maxcoord = sqrt(memlimit / sizeof(DTM));
    if(maxcoord > instance.n1)
      maxcoord = instance.n1;
    msize = (size_t)maxcoord*maxcoord;
    matrix = new DTM[msize];
    for(size_t i = 0; i < msize; i++)
      matrix[i] = std::numeric_limits<DTM>::max();
  }

  DT calculateCostDiff(int i, int j) {
    DT c1 = 0, c2 = 0;
    auto &ai = instance.adj[instance.n0+i];
    auto &aj = instance.adj[instance.n0+j];
    auto itj = aj.begin();

    for(auto iti = ai.begin(); iti != ai.end(); ++iti) {
      bool first = true;
      for(; itj != aj.end(); ++itj) {
        if(first && *itj >= *iti) {
          c2 += itj - aj.begin();
          first = false;
        }
        if(*itj > *iti) {
          c1 += aj.end() - itj;
          break;
        }
      }
      if(first)
        c2 += aj.size();
    }

    return c1-c2;
  }

  DT calculateCostMin(int i, int j) {
    DT c1 = 0, c2 = 0;
    auto &ai = instance.adj[instance.n0+i];
    auto &aj = instance.adj[instance.n0+j];
    const auto aje = aj.end();
    auto itj = aj.begin();

    for(auto iti = ai.begin(); iti != ai.end(); ++iti) {
      bool first = true;
      for(; itj != aje; ++itj) {
        if(first && *itj >= *iti) {
          c2 += itj - aj.begin();
          first = false;
        }
        if(*itj > *iti) {
          c1 += aj.end() - itj;
          break;
        }
      }
      if(first)
        c2 += aj.size();
    }

    return std::min(c1,c2);
  }

  DT costDiff(int i, int j) {
    if(i < maxcoord && j < maxcoord) {
      if(matrixAt(i,j) == std::numeric_limits<DTM>::max()) {
        DT x = calculateCostDiff(i,j);
        matrixAt(i,j) = x;
        matrixAt(j,i) = -x;
        return x;
      }
      return matrixAt(i,j);
    }
    return calculateCostDiff(i,j);
  }

  DTM &matrixAt(int i, int j) {
    return matrix[i + (size_t)maxcoord*j];
  }

  i64 calcLowerBound(double t = -1) {
    i64 ret = 0;
    double t0 = elapsed();

    for(int i = 0; i < instance.n1 - 1; i++) {
      for(int j = i+1; j < instance.n1; j++)
        ret += calculateCostMin(i,j);
      if(t > 0 && elapsed() - t0 > t)
        return ret;
    }
    return ret;
  }

  int median(const std::vector<int> &v) {
    if(v.empty())
      return 0;
    return v[v.size() / 2];
  }

  double average(const std::vector<int> &v) {
    if(v.empty())
      return 0;
    double sum = 0.0;
    for(int i : v)
      sum += i;

    return sum / v.size();
  }

  void solveMedian(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    if(begin + 1 >= end)
      return;

    std::vector<std::tuple<int,double,int>> coords;

    for(auto i = begin; i != end; ++i)
      coords.push_back(std::make_tuple(median(instance.adj.at(instance.n0+*i)), average(instance.adj.at(instance.n0+*i)), *i));

    std::sort(coords.begin(), coords.end());
    for(auto [_,__,i] : coords)
      *begin++ = i;
  }

  void solveAvg(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    if(begin + 1 >= end)
      return;

    std::vector<std::tuple<double,int,int>> coords;

    for(auto i = begin; i != end; ++i)
      coords.push_back(std::make_tuple(average(instance.adj.at(instance.n0+*i)), median(instance.adj.at(instance.n0+*i)), *i));

    std::sort(coords.begin(), coords.end());
    for(auto [_,__,i] : coords)
      *begin++ = i;
  }

  void solveAvgRand(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    if(begin + 1 >= end)
      return;

    std::vector<std::tuple<double,int,int>> coords;
    std::uniform_int_distribution<> distrib(0,0x0fffffff);

    for(auto i = begin; i != end; ++i)
      coords.push_back(std::make_tuple(average(instance.adj.at(instance.n0+*i)),
                                      distrib(rgen),
                                      *i));

    std::sort(coords.begin(), coords.end());
    for(auto [_,__,i] : coords)
      *begin++ = i;
  }

  void solveSplit(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    int n = end - begin;
    if(n <= 1)
      return;
    std::uniform_int_distribution<> distrib(0,n-1);
    auto pivot = begin + distrib(rgen);
    std::swap(*pivot, *begin);
    pivot = begin;
    auto low = begin;
    for(auto p = begin + 1; p != end; ++p) {
      if(costDiff(*p,*pivot) > 0) {
        low++;
        std::swap(*low,*p);
      }
    }
    std::swap(*begin, *low);
    solveSplit(begin, low);
    solveSplit(low+1,end);
  }

  i64 optimizeJumpStep(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    int n = end - begin;
    std::uniform_int_distribution<> distrib(0,n - 1);
    int moving = distrib(rgen);

    std::vector<int> besti = {moving};
    i64 bestChange = 0;
    i64 change = 0;

    for(int t = 1; t < n; t++) {
      if(moving + t == n) {
        change = 0; // Reset for other direction
      }
      int i;
      if(moving + t < n)
        i = moving + t;
      else // Go backwards: decrease i starting at moving-1
        i = n - t - 1;

      if(i > moving)
        change += costDiff(begin[i],begin[moving]);
      else
        change -= costDiff(begin[i],begin[moving]);
  ;
      if(change > bestChange) {
        bestChange = change;
        besti.clear();
      }
      if(change == bestChange) {
        besti.push_back(i);
      }
    }

    std::uniform_int_distribution<> distrib2(0,besti.size()-1);
    int choseni = besti[distrib2(rgen)];
    if(moving <= choseni)
      for(int i = moving; i < choseni; i++)
        std::swap(begin[i], begin[i+1]);
    else
      for(int i = moving; i > choseni; i--)
        std::swap(begin[i], begin[i-1]);

    return bestChange;
  }

  i64 optimizeJump(std::vector<int>::iterator begin, std::vector<int>::iterator end) {
    int n = end - begin;
    i64 ret = 0;
    while(true) {
      i64 improvement = 0;
      for(int i = 0; i < std::min(n,5000); i++) {
        if(n < 17000)
          improvement += optimizeJumpStep(begin, end);
        else { // For large instances, use only a slice to gain speed
          std::uniform_int_distribution<> centerdist(0,n - 1);
          int center = centerdist(rgen);
          int radius = 5000;
          improvement += optimizeJumpStep(
            begin + std::max(0, center - radius),
            std::min(end, begin + center + radius));
        }
      }
      if(improvement == 0)
        break;
      ret += improvement;
    }
    return ret;
  }

  i64 timedOptimizeJump(std::vector<int>::iterator begin, std::vector<int>::iterator end, double t) {
    i64 improvement = 0;
    double t0 = elapsed();
    while(elapsed() - t0 <= t)
      improvement += optimizeJumpStep(begin, end);

    return improvement;
  }

  i64 optimizeSlice(std::vector<int>::iterator begin, std::vector<int>::iterator end, bool jump) {
    i64 improvement = 0;
    std::uniform_int_distribution<> radiusdist(8,18);
    if(!jump)
      radiusdist = std::uniform_int_distribution<>(8,30);
    int radius = radiusdist(rgen);
    radius = radius * radius;

    std::uniform_int_distribution<> centerdist(0,end - begin - 1);
    int center = centerdist(rgen);
    Order s(begin + std::max(0, center - radius),
            std::min(end, begin + center + radius));
    i64 crs = crossings(instance, s.begin(), s.end());

    solveSplit(s.begin(), s.end());

    i64 crs2 = crossings(instance, s.begin(), s.end());
    if(jump) {
      while(true) {
        i64 delta = optimizeJump(s.begin(), s.end());
        if(delta == 0)
          break;
        crs2 -= delta;
      }
    }

    if(crs2 <= crs) {
      for(auto sit = s.begin(), it = begin + std::max(0, center - radius);
          sit != s.end(); ++it, ++sit) {
        *it = *sit;
      }
      improvement = crs - crs2;
    }

    return improvement;
  }
};
