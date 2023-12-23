#ifndef TSPRD_DIVIDEANDSWAP_H
#define TSPRD_DIVIDEANDSWAP_H

#include <vector>

#include "../Data.hpp"
#include "../Solution.h"
#include "InterSearch.hpp"

class DivideAndSwap : public InterSearch {
   private:
    // try to insert a depot in a route, and reorder the routes per release time
    bool divideAndSwapIt(Solution* s) {
        for (int r = 1; r < s->routes.size(); r++) {
            // if the ending time of the previous route is higher than the current route release date
            // it's not possible to improve the ending time of the current route by adding a depot
            // because the starting time of the newly generated route can't be less than the current route start time
            if ((s->routeStart[r - 1] + s->routeTime[r - 1]) > s->routeRD[r]) continue;
            std::vector<int>* route = s->routes[r];

            int maxRD = 0;
            int iMax;
            // find the vertex with higher release date to try to insert depot only after it
            for (int i = F(route); i <= (int)L(route); i++) {
                int rdi = data.releaseDates[route->at(i)];
                if (rdi > maxRD) {
                    maxRD = rdi;
                    iMax = i;
                }
            }

            std::vector<int> totalTimeForward(route->size());  // total time of going from the depot to the i-th element
            totalTimeForward[0] = data.timesMatrix[0][route->at(0)];
            for (int i = 1; i < route->size(); i++)
                totalTimeForward[i] = totalTimeForward[i - 1] + data.timesMatrix[route->at(i - 1)][route->at(i)];

            std::vector<int> totalTimeBack(route->size());  // total time of going from the i-th element to the depot
            std::vector<int> maxRDBack(route->size());      // std::max RD between all element from i to the end
            totalTimeBack.back() = data.timesMatrix[route->back()][0];
            maxRDBack.back() = data.releaseDates[route->back()];
            for (int i = (int)route->size() - 2; i >= 0; i--) {
                totalTimeBack[i] = totalTimeBack[i + 1] + data.timesMatrix[route->at(i)][route->at(i + 1)];
                maxRDBack[i] = std::max(data.releaseDates[route->at(i)], maxRDBack[i + 1]);
            }

            const int rd1 = maxRD;  // the first generated route always have the release date of the original route
            // try to insert depot in each position after the vertex with higher release date
            for (int i = iMax; i < L(route); i++) {
                const int rd2 = maxRDBack[i + 1];  // release date of second route
                const int time1 = totalTimeForward[i] + data.timesMatrix[route->at(i)][0];  // time of the first route
                const int time2 =
                    data.timesMatrix[0][route->at(i + 1)] + totalTimeBack[i + 1];  // time of the second route

                // check if the time improve if we change the original route r(1, N) to the routes r(i+1, N) and R(1, i)
                int time = std::max(s->routeStart[r - 1] + s->routeTime[r - 1], rd2);  // starting time of first route
                time += time2;                                                         // ending time of the first route
                time = std::max(time, rd1);  // starting time of the second route
                time += time1;               // ending time of the second route

                if (time < s->routeStart[r] + s->routeTime[r]) {
                    // if the new route end time is better than the previous route end time
                    // update routes data as needed
                    s->routeRD.insert(s->routeRD.begin() + r, rd2);
                    s->routeTime[r] = time1;
                    s->routeTime.insert(s->routeTime.begin() + r, time2);
                    s->routeStart.push_back(0);  // only increase the size to update after

                    // update routes
                    // move 1 element more in the beginning to change to the depot
                    s->routes.insert(s->routes.begin() + r, new std::vector<int>(make_move_iterator(route->begin() + i),
                                                                                 make_move_iterator(route->end())));

                    s->routes[r + 1]->at(i) = s->routes[r]->at(0);  // restore the moved element
                    s->routes[r]->at(0) = 0;                        // change moved element to depot
                    s->routes[r + 1]->at(i + 1) = 0;                // end depot
                    s->routes[r + 1]->resize(i + 2);                // new route size after moving
                    s->updateStartingTimes(r);

                    return true;
                }
            }
        }

        return false;
    }

   public:
    explicit DivideAndSwap(Data& data) : InterSearch(data) {}

    int search(Solution* solution) {
        int originalTime = solution->time;
        bool improved;
        do {
            improved = divideAndSwapIt(solution);
        } while (improved);
        return originalTime - solution->time;
    }
};

#endif  // TSPRD_DIVIDEANDSWAP_H
