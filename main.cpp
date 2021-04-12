#include <iostream>
#include <algorithm>
#include "Solution.h"
#include "GeneticAlgorithm.h"

using namespace std;

int main(int argc, char **argv) {
    unsigned int mi = 250;
    unsigned int lambda = 1000;
    unsigned int nbElite = 100; // el = 0.4; nbElite = el * mi
    unsigned int nClose = 50;
    unsigned int itNi = 2000; // max iterations without improvement to stop the algorithm
    unsigned int itDiv = 1000; // iterations without improvement to diversify
    unsigned int timeLimit = 10 * 60; // in seconds

    string instanceFile = argv[1];
    Instance instance(instanceFile);

    auto ga = GeneticAlgorithm(instance, mi, lambda, nClose, nbElite, itNi, itDiv, timeLimit);
    Solution s = ga.getSolution();

    cout << "RESULT " << s.time << endl;
    cout << "EXEC_TIME " << ga.getExecutionTime() << endl;
    cout << "SOL_TIME " << ga.getBestSolutionTime() << endl;

    return 0;
}