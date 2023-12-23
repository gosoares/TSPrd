#ifndef TSPRD_INTRATWOOPT_H
#define TSPRD_INTRATWOOPT_H

#include <cassert>

#include "IntraSearch.hpp"

class IntraTwoOpt : public IntraSearch {
   private:
    const std::vector<std::vector<int>>& timesMatrix;

   public:
    explicit IntraTwoOpt(const std::vector<std::vector<int>>& timesMatrix) : timesMatrix(timesMatrix) {}

    /*
     * tenta inverter a ordem de uma subrota que começa no i-ésimo cliente e termina no j-ésimo cliente
     */
    int search(std::vector<int>* route) {
        int bestI, bestJ;
        int bestGain = 0;

        for (int i = 1; i <= L(route) - 1; i++) {
            int minus =
                (int)timesMatrix[route->at(i - 1)][route->at(i)] + (int)timesMatrix[route->at(i)][route->at(i + 1)];
            int plus = 0;
            for (int j = i + 1; j <= L(route); j++) {
                minus += (int)timesMatrix[route->at(j)][route->at(j + 1)];
                plus += (int)timesMatrix[route->at(j)][route->at(j - 1)];

                int gain = minus - (plus + (int)timesMatrix[route->at(i - 1)][route->at(j)] +
                                    (int)timesMatrix[route->at(i)][route->at(j + 1)]);

                if (gain > bestGain) {
                    bestI = i, bestJ = j;
                    bestGain = gain;
                }
            }
        }

        if (bestGain > 0)  // if improved, perform movement
            reverse(route->begin() + bestI, route->begin() + bestJ + 1);

        return bestGain;
    }
};

#endif  // TSPRD_INTRATWOOPT_H
