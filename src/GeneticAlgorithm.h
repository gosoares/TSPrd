#ifndef TSPRD_GENETICALGORITHM_H
#define TSPRD_GENETICALGORITHM_H

#include "Data.hpp"
#include "NeighborSearch.hpp"
#include "Population.h"
#include "Split.h"

class GeneticAlgorithm {
   public:
    Data& data;
    Split split;
    NeighborSearch localSearch;
    Population population;

    GeneticAlgorithm(Data& instance);

    Individual* orderCrossover();

    void diversify();
};

#endif  // TSPRD_GENETICALGORITHM_H
