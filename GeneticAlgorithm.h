#ifndef TSPRD_GENETICALGORITHM_H
#define TSPRD_GENETICALGORITHM_H


#include "Instance.h"
#include "NeighborSearch.h"

class GeneticAlgorithm {
private:
    const Instance instance;
    unsigned int mi;
    unsigned int lambda;
    unsigned int nClose;
    double nbElite;
    unsigned int itNi; // max number of iterations without improvement to stop the algorithm
    unsigned int itDiv; // max number of iterations without improvement to diversify the current population

    NeighborSearch ns;
    Solution *solution;

    vector<Sequence *> *initializePopulation();
    vector<double> getBiasedFitness(vector<Solution *> *solutions) const;
    vector<unsigned int> selectParents(vector<double> &biasedFitness) const;
    static Sequence *orderCrossover(const Sequence &parent1, const Sequence &parent2);
    void survivalSelection(vector<Solution *> *solutions, unsigned int Mi);
    void survivalSelection(vector<Solution *> *solutions) { // default mi
        return survivalSelection(solutions, this->mi);
    }
    void diversify(vector<Solution *> *solutions);
public:
    GeneticAlgorithm(const string &instanceFile, unsigned int mi, unsigned int lambda, unsigned int nClose,
                     double nbElite, unsigned int itNi, unsigned int itDiv);
    const Solution& getSolution() {
        return *solution;
    };
};


#endif //TSPRD_GENETICALGORITHM_H
