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
    vector<Sequence *> *initializePopulation(unsigned int min_pop_size, unsigned int max_pop_size);
    static void getBiasedFitness(vector<double> &biasedFitness, vector<Solution *> *solutions, int min_pop_size, int max_pop_size, int n_close);
    static void survivalSelection(vector<Solution *> *solutions, int min_pop_size, int max_pop_size, int n_close);
};


#endif //TSPRD_FUNCTIONS_H
