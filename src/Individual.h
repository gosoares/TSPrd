#ifndef TSPRD_INDIVIDUAL_H
#define TSPRD_INDIVIDUAL_H

#include "Data.hpp"

class Individual {
   public:
    int eval;                                               // total time of the solution
    std::vector<int> giantTour;                             // giant tour representation (chromossome)
    std::vector<int> successors;                            // for each client (1:V), the successor in the solution
    std::vector<int> predecessors;                          // for each client (1:V), the predecessor in the solution
    std::multiset<std::pair<double, Individual*>> closest;  // Other individuals sorted by distance
    double biasedFitness;                                   // this solution biased fitness

    Individual(Data& data)  // create a random individual only with the giant tour
        : eval(INF), giantTour(data.N), successors(data.V), predecessors(data.V), closest(),
          biasedFitness(std::numeric_limits<double>::max()) {
        std::iota(giantTour.begin(), giantTour.end(), 1);
        std::shuffle(giantTour.begin(), giantTour.end(), data.generator);
    }

    Individual(int N)  // create a empty individual
        : eval(INF), giantTour(N), successors(N + 1), predecessors(N + 1), closest(),
          biasedFitness(std::numeric_limits<double>::max()) {}
};

#endif  // TSPRD_INDIVIDUAL_H
