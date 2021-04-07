#include <iostream>
#include "Solution.h"
#include "GeneticAlgorithm.h"
#include <algorithm>
#include <chrono>

using namespace std;

int main(int argc, char **argv) {
    unsigned int mi = 25;
    unsigned int lambda = 100;
    unsigned int nbElite = 10; // el = 0.4; nbElite = el * mi
    unsigned int nClose = 5;
    unsigned int itNi = 200; // max iterations without improvement to stop the algorithm
    unsigned int itDiv = 100; // iterations without improvement to diversify

    string instanceFile = argv[1];
    Instance instance(instanceFile);

    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto ga = GeneticAlgorithm(instance, mi, lambda, nClose, nbElite, itNi, itDiv);
    Solution s = ga.getSolution();
    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cout << "RESULT " << s.time << endl;
    cout << "TIME " << chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << endl;

    return 0;
}