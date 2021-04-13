#include <iostream>
#include <algorithm>
#include "Solution.h"
#include "GeneticAlgorithm.h"

using namespace std;

int main(int argc, char **argv) {
    unsigned int mi = 25;
    unsigned int lambda = 100;
    unsigned int nbElite = 10; // el = 0.4; nbElite = el * mi
    unsigned int nClose = 5;
    unsigned int itNi = 2000; // max iterations without improvement to stop the algorithm
    unsigned int itDiv = 100; // iterations without improvement to diversify
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