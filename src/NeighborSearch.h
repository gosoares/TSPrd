#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H

#include "InterSearch.h"
#include "IntraSearch.h"
#include "Population.h"

class NeighborSearch {
   private:
    Data& data;

    IntraSearch intraSearch;
    InterSearch interSearch;

    int splitNs(Solution* solution);

   public:
    explicit NeighborSearch(Data& instance);
    int educate(Individual& indiv);
};

#endif  // TSPRD_NEIGHBORSEARCH_H
