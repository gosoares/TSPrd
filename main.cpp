#include <iostream>
#include "Solution.h"
#include "GeneticAlgorithm.h"
#include <algorithm>
#include <chrono>

using namespace std;

int main(int argc, char **argv) {
    int mi = 25;
    int lambda = 100;
    int nClose = 5;
    double nbElite = 0.4;
    int itNi = 200; // max iterations without improvement to stop the algorithm
    int itDiv = 100; // iterations without improvement to diversify
    string instanceFile = argv[1];

    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto ga = GeneticAlgorithm(instanceFile, mi, lambda, nClose, nbElite, itNi, itDiv);
    Solution s = ga.getSolution();
    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cout << "RESULT " << s.time << endl;
    cout << "TIME " << chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << endl;

    return 0;
}