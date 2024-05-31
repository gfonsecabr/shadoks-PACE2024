#pragma once
#include "solution.hpp"
#include <map>
#include <set>

Order *global_best = nullptr;
i64 global_crossings;
int global_nbest = 0;
Instance instance;
bool showProgress = false;


class SolPool {
  std::vector<std::pair<Order,i64>> solutions;
  std::map<i64,std::set<int>> crossMap;

public:
  int insert(Order &sol, i64 cr = -1) {
    if(cr < 0)
      cr = crossings(instance, sol.begin(), sol.end());
    int index = solutions.size();
    solutions.push_back(std::make_pair(sol,cr));
    crossMap[cr].insert(index);
    updateGlobal(sol, cr);
    return index;
  }

  void update(int index, i64 improvement) {
    i64 &cr = solutions.at(index).second;
    crossMap[cr].erase(index);
    if(crossMap[cr].empty())
      crossMap.erase(cr);
    cr -= improvement;

    crossMap[cr].insert(index);
    updateGlobal(solutions[index].first, cr);
  }

  std::vector<int> getIndices() const {
    std::vector<int> indices;
    for(auto &[_,iset] : crossMap) {
      std::vector<int> v(iset.begin(),iset.end());
      std::shuffle(v.begin(), v.end(), rgen);
      indices.insert(indices.end(), v.begin(), v.end());
    }
    return indices;
  }

  Order &operator[](int index) {
    return solutions.at(index).first;
  }

protected:
  void updateGlobal(Order &sol, i64 cr) const {
    if(global_best == nullptr || cr <= global_crossings) {
      auto tmp = global_best;
      global_best = new Order(sol);
      global_crossings = cr;
      global_nbest = crossMap.begin()->second.size();
      delete tmp;
      if(showProgress) {
        std::cout << " -> " << global_crossings;
        if(global_nbest > 1)
          std::cout << "(" << global_nbest << ")";
        std::cout << std::flush;
      }
      // Uncomment to output progress stats
      // if(showProgress) {
      //   std::cerr << elapsed() << " " << global_crossings << std::endl;
      // }
    }
  }
};


