#ifndef TSPRD_FUNCTIONS_H
#define TSPRD_FUNCTIONS_H

#include <vector>
#include "Solution.h"
#include <random>

using namespace std;

class Functions {

public:
    static void orderCrossover(const vector<unsigned int> &parent1, const vector<unsigned int> &parent2, vector<unsigned int> &child);
    static double solutionsDistance(const Solution &s1, const Solution &s2);
    static void initializePopulation(const Instance &instance, vector<vector<Solution *> > &population, unsigned int N);
    static void survivalSelection(vector<vector<Solution> > &population, int pop_size);
};


#endif //TSPRD_FUNCTIONS_H
