#include "InterSearch.h"

#include "Rng.h"
#include "Split.h"

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route
#define N_INTER_SEARCHES 3

InterSearch::InterSearch(Data& data) : data(data), routesPair(), searchOrder(N_INTER_SEARCHES) {
    iota(searchOrder.begin(), searchOrder.end(), 1);
}

int InterSearch::search(Solution* solution) {
    if (solution->routes.size() == 1) return 0;

    int oldTime = solution->time;
    shuffleSearchOrder();

    int whichSearch = 0;
    bool improved = false;
    while (whichSearch < searchOrder.size()) {
        int gain = callInterSearch(solution, searchOrder[whichSearch]);

        if (gain > 0) {
            improved = true;
            if (solution->routes.size() == 1) break;
            int lastMovement = searchOrder[whichSearch];
            shuffleSearchOrder();
            whichSearch = 0;
            if (searchOrder[0] == lastMovement) {
                std::swap(searchOrder[0], searchOrder[searchOrder.size() - 1]);
            }
        } else {
            whichSearch++;
        }
    }

    if (improved) {
        int newTime = solution->update();
        return oldTime - newTime;
    }

    return 0;
}

int InterSearch::callInterSearch(Solution* solution, int which) {
    switch (which) {
        case 1:
            return vertexRelocation(solution);
        case 2:
            return interSwap(solution);
        case 3:
            return insertDepotAndReorder(solution);
        default:
            std::cout << "ERROR invalid_neighbor_search_id" << std::endl;
            exit(1);
    }
}

std::vector<std::pair<int, int> > InterSearch::getRoutesPairSequence(int nRoutes) {
    std::vector<std::pair<int, int> > sequence(nRoutes * nRoutes);
    sequence.resize(0);  // resize but keep allocated space
    for (int i = 0; i < nRoutes; i++) {
        for (int j = i + 1; j < nRoutes; j++) {
            sequence.emplace_back(i, j);
        }
    }
    shuffle(sequence.begin(), sequence.end(), Rng::getGenerator());
    return sequence;
}

int InterSearch::vertexRelocation(Solution* solution) {
    const int originalTime = solution->time;

    int gain;
    do {
        gain = 0;
        for (auto& routePair : getRoutesPairSequence(solution->routes.size())) {
            auto& r1 = routePair.first;
            auto& r2 = routePair.second;
            int gainIt;
            do {
                gainIt = vertexRelocationIt(solution, r1, r2);
                gainIt += vertexRelocationIt(solution, r2, r1);
                gain += gainIt;
            } while (gainIt > 0);
        }
    } while (gain > 0);

    solution->removeEmptyRoutes();
    return originalTime - solution->time;
}

int InterSearch::vertexRelocationIt(Solution* solution, int r1, int r2) {
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
        int r1Time = std::numeric_limits<int>::max();
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

int InterSearch::interSwap(Solution* solution) {
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

int InterSearch::interSwapIt(Solution* solution, int r1, int r2) {
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
            const int r1Time =
                preR1Time + data.timesMatrix[route1->at(i - 1)][vertex2] + data.timesMatrix[vertex2][route1->at(i + 1)];

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

int InterSearch::insertDepotAndReorder(Solution* solution) {
    int originalTime = solution->time;
    bool improved;
    do {
        improved = insertDepotAndReorderIt(solution);
    } while (improved);
    return originalTime - solution->time;
}

// try to insert a depot in a route, and reorder the routes per release time
bool InterSearch::insertDepotAndReorderIt(Solution* s) {
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
            const int rd2 = maxRDBack[i + 1];                                           // release date of second route
            const int time1 = totalTimeForward[i] + data.timesMatrix[route->at(i)][0];  // time of the first route
            const int time2 = data.timesMatrix[0][route->at(i + 1)] + totalTimeBack[i + 1];  // time of the second route

            // check if the time improve if we change the original route r(1, N) to the routes r(i+1, N) and R(1, i)
            int time = std::max(s->routeStart[r - 1] + s->routeTime[r - 1], rd2);  // starting time of first route
            time += time2;                                                         // ending time of the first route
            time = std::max(time, rd1);                                            // starting time of the second route
            time += time1;                                                         // ending time of the second route

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

void InterSearch::shuffleSearchOrder() { shuffle(searchOrder.begin(), searchOrder.end(), Rng::getGenerator()); }
// calculate the new ending time of route std::max(r1, r2) given that r1 and r2 changed
int InterSearch::calculateEndingTime(Solution* solution, int r1, int r2) {
    if (r1 > r2) std::swap(r1, r2);
    // check ending time of route previous to r1
    int time = r1 == 0 ? 0 : solution->routeStart[r1 - 1] + solution->routeTime[r1 - 1];

    for (int i = r1; i <= r2; i++) {
        time = std::max(time, solution->routeRD[i]);  // route starting time
        time += solution->routeTime[i];               // route ending time
    }
    return time;
}

// calculate the release date of the route 'r' in solution when removing 'vertex'
int InterSearch::routeReleaseDateRemoving(Solution* s, int r, int vertex) {
    int rd = s->routeRD[r];

    if (data.releaseDates[vertex] == rd) {  // possibly removing the vertex with bigger RD in the route
        rd = 0;
        std::vector<int>* route = s->routes[r];
        for (int j = F(route); j <= L(route); j++) {
            if (route->at(j) == vertex) continue;
            int rdj = data.releaseDates[route->at(j)];
            if (rdj > rd) rd = rdj;
        }
    }

    return rd;
}

/*
 * Calculate the ending time gain in the ending time of std::max(r1, r2)
 * given that the (release time, route time) ou routes r1 and r1
 * have changed to (r1RD, r1Time) and (r2RD, r2Time) respectively
 *
 * if the ending time is improved, change the given values in the solution,
 * if not, keep the original values
 */
int InterSearch::verifySolutionChangingRoutes(Solution* solution, int r1, int r2, int r1RD, int r1Time, int r2RD,
                                              int r2Time) {
    // calculate the ending time of the bigger route in (r1, r2)
    int br = std::max(r1, r2);
    int originalTime = solution->routeStart[br] + solution->routeTime[br];

    // calculate the solution time, given the changes
    std::swap(r1RD, solution->routeRD[r1]);
    std::swap(r1Time, solution->routeTime[r1]);
    std::swap(r2RD, solution->routeRD[r2]);
    std::swap(r2Time, solution->routeTime[r2]);
    int newTime = calculateEndingTime(solution, r1, r2);

    if (newTime >= originalTime) {  // if not improved, put the values back
        std::swap(r1RD, solution->routeRD[r1]);
        std::swap(r1Time, solution->routeTime[r1]);
        std::swap(r2RD, solution->routeRD[r2]);
        std::swap(r2Time, solution->routeTime[r2]);

        return 0;
    }

    return originalTime - newTime;
}
