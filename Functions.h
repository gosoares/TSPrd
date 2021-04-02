#ifndef TSPRD_FUNCTIONS_H
#define TSPRD_FUNCTIONS_H

#include <vector>
#include "Solution.h"
#include <random>

using namespace std;

class Functions {
    const Instance &instance;
public:
    explicit Functions(const Instance &instance): instance(instance) {};

    static Sequence *orderCrossover(const Sequence &parent1, const Sequence &parent2);
    static double routesDistance(Solution *r1, Solution *r2);
    vector<Sequence *> *initializePopulation(unsigned int mi, unsigned int lambda = 0);
    static void getBiasedFitness(vector<double> &biasedFitness, vector<Solution *> *solutions, double nbElite, int nClose);
    static void survivalSelection(vector<Solution *> *solutions, int mi, double nbElite, int nClose);
    void diversify(vector<Solution *> *solutions, int mi, double nbElite, int nClose);
};


#endif //TSPRD_FUNCTIONS_H
