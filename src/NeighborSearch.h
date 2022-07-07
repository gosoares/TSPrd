#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H

#include "InterSearch.h"
#include "IntraSearch.h"
#include "Solution.h"

class NeighborSearch {
   private:
    Data& data;

    IntraSearch intraSearch;
    InterSearch interSearch;

    int splitNs(Solution* solution);

   public:
    explicit NeighborSearch(Data& instance);
    int educate(Solution* solution);
};

#endif  // TSPRD_NEIGHBORSEARCH_H
