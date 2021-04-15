#ifndef TSPRD_GRASP_H
#define TSPRD_GRASP_H

#include "Solution.h"

using namespace chrono;

class Grasp {
    const Instance &instance;
    const vector<vector<unsigned int> > &W;
    const vector<unsigned int> &RD;
    unsigned int itNi; // iterations without improvement to stop algorithm
    const double alpha;
    const unsigned int timeLimit;

    Solution *bestSolution;

    steady_clock::time_point beginTime;
    steady_clock::time_point endTime;
    steady_clock::time_point bestSolutionFoundTime;

    Solution *constructSolution();

public:
    explicit Grasp(const Instance &instance, unsigned int itNi, double alpha, unsigned int timeLimit);

    const Solution &getSolution() {
        return *bestSolution;
    }

    unsigned int getExecutionTime() {
        return duration_cast<milliseconds>(endTime - beginTime).count();
    }
    unsigned int getBestSolutionTime() {
        return duration_cast<milliseconds>(bestSolutionFoundTime - beginTime).count();
    }
};

#endif //TSPRD_GRASP_H
