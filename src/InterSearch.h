#ifndef TSPRD_INTERSEARCH_H
#define TSPRD_INTERSEARCH_H

#include "Data.hpp"
#include "Solution.h"

class InterSearch {
   private:
    Data& data;

    std::vector<std::vector<std::pair<int, int> > > routesPair;
    std::vector<int> searchOrder;

    int callInterSearch(Solution* solution, int which);
    int vertexRelocation(Solution* solution);
    int vertexRelocationIt(Solution* solution, int r1, int r2);
    int interSwap(Solution* solution);
    int interSwapIt(Solution* solution, int r1, int r2);
    int insertDepotAndReorder(Solution* solution);
    bool insertDepotAndReorderIt(Solution* s);

    std::vector<std::pair<int, int> > getRoutesPairSequence(int nRoutes);
    static int calculateEndingTime(Solution* solution, int r1, int r2);
    int routeReleaseDateRemoving(Solution* s, int r, int vertex);
    static int verifySolutionChangingRoutes(Solution* solution, int r1, int r2, int r1RD, int r1Time, int r2RD,
                                            int r2Time);

    void shuffleSearchOrder();

   public:
    explicit InterSearch(Data& data);
    int search(Solution* solution);
};

#endif  // TSPRD_INTERSEARCH_H
