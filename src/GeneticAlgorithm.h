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

    // stores (time, value) of each best solution found
    std::vector<std::pair<std::chrono::steady_clock::time_point, int>> searchProgress;
    std::uniform_int_distribution<int> distPopulation;  // distribution for the population [0, mi)

    std::vector<Sequence*>* initializePopulation();

    std::vector<double> getBiasedFitness(std::vector<Solution*>* solutions) const;

    std::vector<int> selectParents(std::vector<double>& biasedFitness);

    static double solutionsDistances(Solution* s1, Solution* s2, bool symmetric);

    Sequence* orderCrossover(const Sequence& parent1, const Sequence& parent2);

    void survivalSelection(std::vector<Solution*>* solutions, int Mi);

    void survivalSelection(std::vector<Solution*>* solutions) {  // default mi
        return survivalSelection(solutions, this->data.params.mu);
    }

    void diversify(std::vector<Solution*>* solutions);

   public:
    GeneticAlgorithm(Data& instance);

    const Solution& getSolution() { return *bestSolution; };

    int getExecutionTime() { return endTime.count(); }

    int getBestSolutionTime() { return bestSolutionFoundTime.count(); }

    const std::vector<std::pair<std::chrono::steady_clock::time_point, int>>& getSearchProgress() {
        return searchProgress;
    }
};

#endif  // TSPRD_GENETICALGORITHM_H
