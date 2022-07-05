#include "NeighborSearch.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <limits>

#include "Split.h"

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route

NeighborSearch::NeighborSearch(const Instance& instance)
    : instance(instance), W(instance.getW()), RD(instance.getRD()), generator((random_device())()),
      intraSearch(instance), interSearch(instance) {}

unsigned int NeighborSearch::splitNs(Solution* solution) {
    Sequence* sequence = solution->toSequence();
    set<unsigned int> depotVisits;
    unsigned int splitTime = Split::split(depotVisits, instance.getW(), instance.getRD(), *sequence);

    unsigned int gain = 0;
    if (splitTime < solution->time) {
        gain = solution->time - splitTime;
        Solution newSolution(instance, *sequence, &depotVisits);
        solution->mirror(&newSolution);
    }
    delete sequence;
    return gain;
}

unsigned int NeighborSearch::educate(Solution* solution) {
    const unsigned int originalTime = solution->time;

    intraSearch.search(solution);

    int which = 1;  // 0: intra   1: inter
    bool splitImproved;
    do {
        unsigned int gain;
        do {
            gain = (which == 0) ? intraSearch.search(solution) : interSearch.search(solution);
            which = 1 - which;
        } while (gain > 0);

        splitImproved = splitNs(solution) > 0;
    } while (splitImproved);

    return originalTime - solution->time;
}
