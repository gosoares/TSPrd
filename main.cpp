#include <iostream>
#include <algorithm>
#include "Solution.h"
#include "GeneticAlgorithm.h"
#include "Grasp.h"

using namespace std;

int main(int argc, char **argv) {

    // genetic algorithm parameters
    unsigned int mi = 25;
    unsigned int lambda = 100;
    unsigned int nbElite = 10; // el = 0.4; nbElite = el * mi
    unsigned int nClose = 5;
    unsigned int itNi = 2000; // max iterations without improvement to stop the algorithm
    unsigned int itDiv = 100; // iterations without improvement to diversify

    // grasp parameters
    unsigned int itNiGrasp = 1000;
    double alpha = 0.2;

    unsigned int timeLimit = 10 * 60; // in seconds

    string instanceFile = argv[1];
    Instance instance(instanceFile);

    auto alg = GeneticAlgorithm(instance, mi, lambda, nClose, nbElite, itNi, itDiv, timeLimit);
//    auto alg = Grasp(instance, itNiGrasp, alpha, timeLimit);
    Solution s = alg.getSolution();
    s.validate();

    cout << "RESULT " << s.time << endl;
    cout << "EXEC_TIME " << alg.getExecutionTime() << endl;
    cout << "SOL_TIME " << alg.getBestSolutionTime() << endl;

    return 0;
}