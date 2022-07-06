#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H

#include "Instance.h"
#include "InterSearch.h"
#include "IntraSearch.h"
#include "Solution.h"

class NeighborSearch {
   private:
    const Instance& instance;
    const vector<vector<unsigned int> >& W;
    const vector<unsigned int>& RD;

    InterSearch interSearch;
    IntraSearch intraSearch;

    unsigned int splitNs(Solution* solution);

   public:
    explicit NeighborSearch(const Instance& instance);
    unsigned int educate(Solution* solution);
};

#endif  // TSPRD_NEIGHBORSEARCH_H
