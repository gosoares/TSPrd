#ifndef TSPRD_INTERSWAP_H
#define TSPRD_INTERSWAP_H

#include <vector>

#include "../Data.hpp"
#include "../Solution.h"
#include "InterSearch.hpp"

class InterSwap : public InterSearch {
   private:
    int interSwapIt(Solution* solution, int r1, int r2) {
        std::vector<int>* route1 = solution->routes[r1];
        std::vector<int>* route2 = solution->routes[r2];

        // try to swap the i-th vertex from r1 with the j-th vertex from r2
        for (int i = F(route1); i <= (int)L(route1); i++) {
            const int vertex1 = route1->at(i);

            // check the new release date of route1 when removing 'vertex1'
            const int preR1RD = routeReleaseDateRemoving(solution, r1, vertex1);

            // time of the route without the arcs with vertex1
            const int preR1Time = solution->routeTime[r1] - data.timesMatrix[route1->at(i - 1)][vertex1] -
                                  data.timesMatrix[vertex1][route1->at(i + 1)];

            // check where to put vertex to have the smaller route time
            for (int j = F(route2); j <= L(route2); j++) {
                const int vertex2 = route2->at(j);
                const int r1RD = std::max(data.releaseDates[vertex2], preR1RD);
                const int r1Time = preR1Time + data.timesMatrix[route1->at(i - 1)][vertex2] +
                                   data.timesMatrix[vertex2][route1->at(i + 1)];

                int r2RD = routeReleaseDateRemoving(solution, r2, vertex2);  // removing vertex2
                r2RD = std::max(r2RD, data.releaseDates[vertex1]);           // inserting vertex1
                const int r2Time = solution->routeTime[r2] - data.timesMatrix[route2->at(j - 1)][vertex2] -
                                   data.timesMatrix[vertex2][route2->at(j + 1)] +
                                   data.timesMatrix[route2->at(j - 1)][vertex1] +
                                   data.timesMatrix[vertex1][route2->at(j + 1)];

                const int routeGain = verifySolutionChangingRoutes(solution, r1, r2, r1RD, r1Time, r2RD, r2Time);
                if (routeGain > 0) {  // perform movement
                    std::swap(route1->at(i), route2->at(j));
                    solution->updateStartingTimes(std::min(r1, r2));
                    return routeGain;
                }
            }
        }

        return 0;
    }

   public:
    explicit InterSwap(Data& data) : InterSearch(data) {}

    int search(Solution* solution) override {
        const int originalTime = solution->time;
        int gain;
        do {
            gain = 0;
            for (auto& routePair : getRoutesPairSequence(solution->routes.size())) {
                int gainIt;
                do {
                    gainIt = interSwapIt(solution, routePair.first, routePair.second);
                    gain += gainIt;
                } while (gainIt > 0);
            }
        } while (gain > 0);
        return originalTime - solution->time;
    }
};

#endif  // TSPRD_INTERSWAP_H
