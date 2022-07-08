#include "NeighborSearch.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <limits>

#include "Split.h"

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route

NeighborSearch::NeighborSearch(Data& data) : data(data), intraSearch(data), interSearch(data) {}

int NeighborSearch::splitNs(Solution* solution) {
    Sequence* sequence = solution->toSequence();
    std::set<int> depotVisits;
    int splitTime = Solution::split(depotVisits, data.timesMatrix, data.releaseDates, *sequence);

    int gain = 0;
    if (splitTime < solution->time) {
        gain = solution->time - splitTime;
        Solution newSolution(data, *sequence, &depotVisits);
        solution->mirror(&newSolution);
    }
    delete sequence;
    return gain;
}

int NeighborSearch::educate(Individual* indiv) {
    const int originalTime = indiv->eval;

    Solution* solution = new Solution(data, indiv->giantTour, nullptr);

    int which = 1;  // 0: intra   1: inter
    bool splitImproved;
    do {
        int gain;
        do {
            gain = (which == 0) ? intraSearch.search(solution) : interSearch.search(solution);
            which = 1 - which;
        } while (gain > 0);

        splitImproved = splitNs(solution) > 0;
    } while (splitImproved);

    int i = 0;
    for (const std::vector<int>* route : solution->routes) {
        for (int j = 1; j < route->size() - 1; j++) {
            indiv->giantTour[i] = route->at(j);
            i++;
        }
    }
    auto improved = originalTime - solution->time;
    delete solution;

    return improved;
}
