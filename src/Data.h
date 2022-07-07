#ifndef TSPRD_DATA_H
#define TSPRD_DATA_H

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "Instance.h"

// parameters for the algorithm
struct AlgParams {
    int mu;         // minimum size of the population
    int lambda;     // maximum number of additional individuals in the population
    int nbElite;    // number of elite individuals for the biased fitness
    int nClose;     // number of closest indivials to consider when calculating the diversity
    int itNi;       // max iterations without improvement to stop the algorithm
    int itDiv;      // iterations without improvement to diversify
    int timeLimit;  // in seconds

    long long seed;  // seed for RNG
};

class Data {
   public:
    // instance data
    const int V, N;                                    // how many vertices and how many clients (V-1 )
    const std::vector<std::vector<int>>& timesMatrix;  // times matrix
    const std::vector<int>& releaseDates;              // release date of each vertex
    const int biggerReleaseDate;                       // the bigger release date in the instance
    const bool symmetric;                              // whether the time matrix is simmetric

    // parameters for the algorithm
    const AlgParams& params;

    // starting time of the algorithm
    const std::chrono::steady_clock::time_point startTime;

    // for RNG
    std::mt19937 generator;

    explicit Data(const Instance& filename, const AlgParams& params);
    std::chrono::milliseconds elapsedTime() const;
};

#endif  // TSPRD_DATA_H
