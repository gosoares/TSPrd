#ifndef TSPRD_GENETICALGORITHM_H
#define TSPRD_GENETICALGORITHM_H


#include <chrono>
#include "Instance.h"
#include "NeighborSearch.h"
#include "Timer.h"
#include "RoutePool.h"

using namespace chrono;

class GeneticAlgorithm {
private:
    const Instance &instance;
    const unsigned int mi; // minimum size of the population
    const unsigned int lambda; // how much individuals will be generated from the mi individuals
    // max population size = mi + lambda
    const unsigned nbElite; // number of elite individuals (in terms of time) to survive the next generation
    const unsigned int nClose; // number of closest solutions to consider when calculating the nCloseMean
    const unsigned int itNi; // max number of iterations without improvement to stop the algorithm
    const unsigned int itDiv; // max number of iterations without improvement to diversify the current population
    const unsigned int timeLimit; // time limit of the execution of the algorithm in seconds

    NeighborSearch ns;
    Solution *bestSolution;

    Timer<milliseconds, steady_clock> timer;
    chrono::milliseconds endTime;
    chrono::milliseconds bestSolutionFoundTime;

    vector<pair<unsigned int, unsigned int> > searchProgress; // stores (time, value) of each best solution found

    RoutePool &routePool;

    // random number generator
    mt19937 generator;
    uniform_int_distribution<int> distPopulation; // distribution for the population [0, mi)

    vector<Sequence *> *initializePopulation();

    vector<double> getBiasedFitness(vector<Solution *> *solutions) const;

    vector<unsigned int> selectParents(vector<double> &biasedFitness);

    static double solutionsDistances(Solution *s1, Solution *s2, bool symmetric);

    static Sequence *orderCrossover(const Sequence &parent1, const Sequence &parent2);

    void survivalSelection(vector<Solution *> *solutions, unsigned int Mi);

    void survivalSelection(vector<Solution *> *solutions) { // default mi
        return survivalSelection(solutions, this->mi);
    }

    void diversify(vector<Solution *> *solutions);

public:
    GeneticAlgorithm(const Instance &instance, unsigned int mi, unsigned int lambda, unsigned int nClose,
                     unsigned int nbElite, unsigned int itNi, unsigned int itDiv, unsigned int timeLimit,
                     RoutePool &routePool);

    const Solution &getSolution() {
        return *bestSolution;
    };

    unsigned int getExecutionTime() {
        return endTime.count();
    }

    unsigned int getBestSolutionTime() {
        return bestSolutionFoundTime.count();
    }

    const vector<pair<unsigned int, unsigned int> > &getSearchProgress() {
        return searchProgress;
    }
};


#endif //TSPRD_GENETICALGORITHM_H
