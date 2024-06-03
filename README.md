# shadoks-PACE2024
Code for the [PACE 2024](https://pacechallenge.org/2024/) challenge to solve the one-sided crossing minimization problem (OCM). Each input file contains a bipartite graph with two partitions called `top` and `bottom`. The `top` partition comes with a vertex order. The `output` is the vertex order of the `bottom` partition and the goal is to minimize the number of edge crossings when the vertices of the two partitions are placed on two parallel lines in order and the edges are drawn as line segments.

## Compilation
Preferably run
```
cmake .
make
```
or, if the previous solution seems complicated, simply run
```
./compile.sh
```
This will produce two executable files: `heuristic` and `exact`, corresponding to the two tracks.

## Dependencies

The code needs a C++ compiler that accepts C++20. It uses no library besides the standard library, but contains a modified version of the segment tree from
https://github.com/t3nsor/codebook/blob/master/segtree.cpp in the `segment.hpp` file.

## Execution
There are two ways to execute the program. Preferably run
```
./heuristic input.gr output
```
This will print some messages on the screen so you don't get bored. Alternatively, run
```
./heuristic <input.gr >output
```
which will run the program without any messages. The `exact` program is invoked the same way.

## Behavior
The `heuristic` program runs for at most 5 minutes and then saves the solution to `output`. The exact program runs for at most 30 minutes and may either save the solution to `output` or return an error code in the end. The `exact` code offers no guarantee that the solution is optimal, but will heuristically evaluate the optimality of the solution, returning an error in case the confidence is not high enough. The memory is limited to around 8GB on both programs. Both programs will save the solution and terminate if they receive a `SIGINT` signal or `control-c`.

## Algorithm
See the [description.pdf](description.pdf) file for details about the algorithm. The general idea is the following. We produce a number `nSols` of initial solutions using median, average, and `split` heuristics. The solutions are then improved in (essentially) two different ways:
1. Moving a (randomly chosen) `bottom` vertex to the position that minimizes the number of crossings (which we call `jump`).
2. Randomly choosing an interval of `bottom` vertices and computing a `split` solution to that interval. The new order is kept if the number of crossings does not increase.

The confidence is determined by the number of solutions with the best number of crossings divided by `nSols`. In the exact version a confidence of 75% is required. In both versions, we stop the calculation prematurely if the confidence gets to 100% (or if a trivial lower bound is reached).

## Intuition

The `split` heuristic randomly chooses a `bottom` vertex as the `pivot`. It then splits the remaining `bottom` vertices into two sets, depending on whether there are fewer crossings when they are placed to the left or to the right of the pivot (in case of equality, we choose randomly). This heuristic produced fairly good solutions and it has the advantage of being very random, producing very different solutions on each run. Hence, it is a good heuristic to avoid local optima. The `jump` local search step gives good results and can be implemented in time linear on the number of bottom vertices after building a matrix for the instance at preprocessing time. A careful combination and efficient implementation of these two ideas yilded the code herein.

My inability to produce instances where the heuristics often converge to suboptimal solutions with the same number of crossings motivated the submission to the exact track. Whether one can prove that the exact code does not output a suboptimal solution with high probability (depending on the random values used) is an open problem. Such a proof would require a good understanding of the space of solutions connected by some local search operations.

## Parameters
Many parameters are hardcoded. The ones that are more easily modified are declared as constant on the top of `main.cpp`:
+ `memlimit`: Number of bytes of memory to use in the matrix (used to speed up crossing calculations). Almost all memory use comes for this matrix.
+ `maxTime`: Number of seconds to run. The actual execution may take a few seconds more.
+ `nSols`: Number of solutions to improve simultaneously. Set to 12 in the heuristic version and 32 in the exact version.

## Directories
The base directory contain the source code. The `solutions` directory contains the best solutions we found with our solver for the public PACE instances (sometimes after several hours of computation, notably for `h044.sol`). Heuristic-track instances start with `h` and exact-track instances start with `e`.
