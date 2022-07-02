#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H

#include <random>
#include "Instance.h"
#include "Solution.h"
#include "IntraSearch.h"
#include "InterSearch.h"

class NeighborSearch {
private:
    const Instance& instance;
    const vector<vector<unsigned int> > &W;
    const vector<unsigned int> &RD;

    InterSearch interSearch;
    IntraSearch intraSearch;

    mt19937 generator;

    unsigned int splitNs(Solution *solution);
public:
    explicit NeighborSearch(const Instance& instance);
    unsigned int educate(Solution *solution);
};


#endif //TSPRD_NEIGHBORSEARCH_H
