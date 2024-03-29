#ifndef TSPRD_POPULATION_H
#define TSPRD_POPULATION_H

#include "Data.hpp"
#include "Individual.h"
#include "Split.h"

typedef std::vector<Individual*> Individuals;

class Population {
   private:
    Data& data;
    Split split;

   public:
    Individuals individuals;
    Individual bestSolution;  // best solution found
    std::vector<std::pair<int, int>> searchProgress;

    Population(Data& data, Split& split);
    ~Population();

    void initialize();
    bool add(Individual* indiv);
    void survivorsSelection(int nSurvivors = -1);
    void removeWorst();

    void updateBiasedFitness();
    double distance(Individual* indiv1, Individual* indiv2);
    double nCloseMean(Individual* indiv);

    std::pair<Individual*, Individual*> selectParents();
    void diversify();

    size_t size();
};

#endif  // TSPRD_POPULATION_H
