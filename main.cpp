#include "instance.hpp"
#include "solution.hpp"
#include "solpool.hpp"
#include "solvers.hpp"
#include <signal.h>
#include <cstring>
#include <sstream>
#include <variant>

// Memory limit for the matrix
size_t memlimit = (size_t) 8e9;

#ifdef EXACT
double maxTime = 30 * 60 - 10; // Time in seconds before terminating (exact version)
int nSols = 32; // Number of solutions kept
#else
double maxTime = 5 * 60 - 5; // Time in seconds before terminating (heuristic version)
int nSols = 12; // Number of solutions kept
#endif


std::string instfn, solfn;
i64 lowerBound = 0;

void nop(int signum) {
  return;
}

// This function is called to output a solution and finish the program
// It may be called by a signal hander, in this case the behavior is
// unpredictable as it calls non-reentrant functions.
// Normally it seems to work though!
void terminate(int signum = -1) {
  if(signum == -1) { // Make sure a signal does not call this again
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = nop;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
  }

  std::string confidence;
  if(showProgress) {
    if(global_crossings > lowerBound) {
      std::stringstream confidencess;
      confidencess << global_nbest << "/" << nSols;
      confidence = confidencess.str();
    }
    else {
      confidence = "OPTIMAL";
    }
  }

#ifdef EXACT
  if(global_crossings > lowerBound && global_nbest < .75 * nSols) {
    if(showProgress) {
        std::cout << std::endl << "Found solution with " << global_crossings << " crossings but the confidence is too small: " << confidence <<  std::endl;
    }
    _Exit(1);
  }
#endif
  if(global_best != nullptr) {
    if(instfn.empty())
      print(instance, *global_best);
    else {
      save(instance, *global_best, solfn);
      if(solfn.empty())
        std::cout << std::endl << "Found solution with " << global_crossings << " crossings and confidence "  << confidence << " but did not save it because no filename given"<< std::endl;
      else
        std::cout << std::endl << "Saved solution with " << global_crossings << " crossings and confidence "  << confidence << " to " << solfn << std::endl;
    }
    _Exit(0);
  }
  _Exit(1);
}

template<class T>
void fillSolutions(SolPool &solPool, i64 lowerBound, T &solvers) {
  for(int i = 0; i < nSols; i++) {
    Order sol = instance.v1;
    std::shuffle(sol.begin(), sol.end(), rgen);
    if(i == 0)
      solvers.solveAvg(sol.begin(), sol.end());
    else if(i == 1)
      solvers.solveMedian(sol.begin(), sol.end());
#ifndef EXACT
    else if(instance.n1 > 30000)
      solvers.solveAvgRand(sol.begin(), sol.end());
#endif
    else
      solvers.solveSplit(sol.begin(), sol.end());

    i64 cr = crossings(instance, sol.begin(), sol.end());
    solPool.insert(sol);

    if(elapsed() > maxTime || cr <= lowerBound)
      terminate();

    if(elapsed() > maxTime / 5)
      break;
  }
}

int main(int argc, char **argv) {
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = terminate;
  sigaction(SIGTERM, &action, NULL);
  sigaction(SIGINT, &action, NULL);

  if(argc >=2) {
    instfn = argv[1];
    if(!instfn.empty())
      showProgress = true;
  }

  if(argc >=3) {
    solfn = argv[2];
  }

  instance = Instance(instfn);
  using Solversv = std::variant<Solvers<int, short int>, Solvers<int,int>, Solvers<i64,i64>>;
  Solversv solversv;

  // Choose the right version of the Solvers template according to the instance size
  if((i64) instance.v1Degree * instance.v1Degree < std::numeric_limits<short int>::max()) {
    solversv = Solvers<int,short int>(memlimit);
  }
  else if((i64) instance.v1Degree * instance.v1Degree < std::numeric_limits<int>::max()) {
    solversv = Solvers<int,int>(memlimit);
  }
  else {
    solversv = Solvers<i64,i64>(memlimit);
  }

  if(showProgress) {
    std::cout << "Read " << instfn << " with " << instance.n0 << " + " << instance.n1 << "(" << instance.v1.size() << ") vertices, " << instance.m << " edges, " << instance.v1Degree << " max degree " << std::endl;
  }

#ifdef EXACT
  lowerBound = std::visit([](auto&& e){ return e.calcLowerBound(maxTime/2); }, solversv);
  bool exact = true;
#else
  // Do not waste time computing lower bound for large instances in heuristic mode
  lowerBound = std::visit([](auto&& e){ return e.calcLowerBound(3); }, solversv);
  bool exact = false;
#endif

  SolPool solPool;

  if(showProgress) {
    std::cout << "Lower bound: " << lowerBound << std::endl;
    std::cout << "Best number of crossings so far (out of " << nSols << ")";
  }

  std::visit([&solPool](auto&& e){ return fillSolutions(solPool, lowerBound, e); }, solversv);

  int hardInstance = 0;

  // Main loop
  do {
    for(int i : solPool.getIndices()){
      Order &sol = solPool[i];
      i64 improvement = 0;

      if(hardInstance <= 0) {
        // Quick improvement for easy-to-improve instances
        double t = elapsed();
        for(int i = 0; i < 5; i++) {
          improvement += std::visit([&sol](auto&& e){return e.optimizeSlice(sol.begin(), sol.end(), false);}, solversv);
        }
        t = elapsed() - t;

        if(improvement == 0 || exact) {
          improvement += std::visit([&sol, t](auto&& e){return e.timedOptimizeJump(sol.begin(), sol.end(), t/2);}, solversv);
        }
      }

      if(improvement == 0 || exact) {
        // Slower improvement when closer to optimal
        for(int i = 0; i < 5; i++) {
          double t = elapsed();
          improvement += std::visit([&sol](auto&& e){return e.optimizeSlice(sol.begin(), sol.end(), true);}, solversv);
          t = elapsed() - t;

          improvement += std::visit([&sol, t](auto&& e){return e.timedOptimizeJump(sol.begin(), sol.end(), t/2);}, solversv);
        }
      }

      if(improvement > 0) {
        solPool.update(i, improvement);
        break; // To work more time on better solutions
      }
      else {
        hardInstance = 16;
      }
    }
    hardInstance--;
  } while(global_crossings > lowerBound && elapsed() < maxTime && global_nbest < nSols);

  terminate();

  return 2; // Should never be reached
}
