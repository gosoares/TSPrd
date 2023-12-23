#ifndef TSPRD_INTRAREINSERTION_H
#define TSPRD_INTRAREINSERTION_H

#include <cassert>

#include "IntraSearch.hpp"

class IntraReinsertion : public IntraSearch {
   private:
    const std::vector<std::vector<int>>& timesMatrix;
    int n;

   public:
    explicit IntraReinsertion(const std::vector<std::vector<int>>& timesMatrix, int n)
        : timesMatrix(timesMatrix), n(n) {}

    /*
     * Tenta realizar a reinserção de um conjunto de n clientes adjacentes em todas as outras posições possíveis
     *
     * i: índice do primeiro elemento do conjunto
     * i + n - 1: índice do ultimo elemento do conjunto
     * L: índice do ultimo cliente na rota
     *
     * j representa onde será feita a tentativa de reinserção
     */
    int search(std::vector<int>* route) {
        int bestI, bestJ;
        int bestGain = 0;

        for (int i = 1; i + n - 1 <= L(route); i++) {
            int minusFixed = (int)timesMatrix[route->at(i - 1)][route->at(i)] +
                             (int)timesMatrix[route->at(i + n - 1)][route->at(i + n)];
            int plusFixed = (int)timesMatrix[route->at(i - 1)][route->at(i + n)];

            for (int j = 0; j <= L(route); j++) {
                if (j >= i - 1 && j <= i + n - 1) continue;

                int minus = minusFixed + (int)timesMatrix[route->at(j)][route->at(j + 1)];
                int plus = plusFixed + (int)timesMatrix[route->at(j)][route->at(i)] +
                           (int)timesMatrix[route->at(i + n - 1)][route->at(j + 1)];

                int gain = minus - plus;
                if (gain > bestGain) {
                    bestI = i;
                    bestJ = j;
                    bestGain = gain;
                }
            }
        }

        if (bestGain > 0) {  // perform reinsertion
            if (bestI > bestJ) {
                // rotate vertex backwards
                rotate(route->begin() + bestJ + 1, route->begin() + bestI, route->begin() + bestI + n);
            } else {
                // rotate vertex forward
                rotate(route->begin() + bestI, route->begin() + bestI + n, route->begin() + bestJ + 1);
            }
        }
        return bestGain;
    }
};

#endif  // TSPRD_INTRAREINSERTION_H
