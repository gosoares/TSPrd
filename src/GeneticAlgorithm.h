#ifndef TSPRD_GENETICALGORITHM_H
#define TSPRD_GENETICALGORITHM_H

#include "Data.h"
#include "NeighborSearch.h"

class GeneticAlgorithm {
   private:
    Data& data;

    NeighborSearch ns;
    Solution* bestSolution;

    std::chrono::milliseconds endTime;
    std::chrono::milliseconds bestSolutionFoundTime;

    Sequence* orderCrossover(const Sequence& parent1, const Sequence& parent2);

    void diversify(std::vector<Solution*>* solutions);

   public:
    GeneticAlgorithm(Data& instance);

    int getExecutionTime() { return endTime.count(); }

    int getBestSolutionTime() { return bestSolutionFoundTime.count(); }
};

#endif  // TSPRD_GENETICALGORITHM_H
