#ifndef TSPRD_INTERRELOCATION_H
#define TSPRD_INTERRELOCATION_H

#include <vector>

#include "../Solution.h"
#include "InterSearch.hpp"

class InterRelocation : public InterSearch {
   private:
    int interRelocationIt(Solution* solution, int r1, int r2) {
        std::vector<int>* route1 = solution->routes[r1];
        std::vector<int>* route2 = solution->routes[r2];

        // try to remove a vertex from r2 and put in r1
        for (int i = F(route2); i <= L(route2); i++) {
            int vertex = route2->at(i);

            // check the new release date of route2 when removing 'vertex'
            int r2RD = routeReleaseDateRemoving(solution, r2, vertex);

            // calculate the new route time of route2 when removing vertex
            int r2Time = solution->routeTime[r2] - data.timesMatrix[route2->at(i - 1)][route2->at(i)] -
                         data.timesMatrix[route2->at(i)][route2->at(i + 1)] +
                         data.timesMatrix[route2->at(i - 1)][route2->at(i + 1)];

            // check release date of route1, when inserting 'vertex'
            int r1RD = std::max(solution->routeRD[r1], data.releaseDates[vertex]);

            // check where to put vertex to have the smaller route time
            int r1Time = INF;
            int bestJ;
            for (int j = 0; j < route1->size() - 1; j++) {
                int time = solution->routeTime[r1] - data.timesMatrix[route1->at(j)][route1->at(j + 1)] +
                           data.timesMatrix[route1->at(j)][vertex] + data.timesMatrix[vertex][route1->at(j + 1)];
                if (time < r1Time) {
                    r1Time = time;
                    bestJ = j;
                }
            }

            int routeGain = verifySolutionChangingRoutes(solution, r1, r2, r1RD, r1Time, r2RD, r2Time);
            if (routeGain > 0) {                     // perform the movement
                route2->erase(route2->begin() + i);  // delete i-th element
                route1->insert(route1->begin() + bestJ + 1, vertex);
                solution->updateStartingTimes(std::min(r1, r2));

                return routeGain;
            }
        }

        return 0;
    }

   public:
    explicit InterRelocation(Data& data) : InterSearch(data) {}

    int search(Solution* solution) {
        const int originalTime = solution->time;

        int gain;
        do {
            gain = 0;
            for (auto& routePair : getRoutesPairSequence(solution->routes.size())) {
                auto& r1 = routePair.first;
                auto& r2 = routePair.second;
                int gainIt;
                do {
                    gainIt = interRelocationIt(solution, r1, r2);
                    gainIt += interRelocationIt(solution, r2, r1);
                    gain += gainIt;
                } while (gainIt > 0);
            }
        } while (gain > 0);

        solution->removeEmptyRoutes();
        return originalTime - solution->time;
    }
};

#endif  // TSPRD_INTERRELOCATION_H
