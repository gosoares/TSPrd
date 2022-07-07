#ifndef TSPRD_INTERSEARCH_H
#define TSPRD_INTERSEARCH_H

#include "Instance.h"
#include "Solution.h"

class InterSearch {
   private:
    const Instance& instance;
    const vector<vector<unsigned int> >& W;
    const vector<unsigned int>& RD;

    vector<vector<pair<unsigned int, unsigned int> > > routesPair;
    vector<unsigned int> searchOrder;

    unsigned int callInterSearch(Solution* solution, unsigned int which);
    unsigned int vertexRelocation(Solution* solution);
    unsigned int vertexRelocationIt(Solution* solution, unsigned int r1, unsigned int r2);
    unsigned int interSwap(Solution* solution);
    unsigned int interSwapIt(Solution* solution, unsigned int r1, unsigned int r2);
    unsigned int insertDepotAndReorder(Solution* solution);
    bool insertDepotAndReorderIt(Solution* s);

    vector<pair<unsigned int, unsigned int> > getRoutesPairSequence(unsigned int nRoutes);
    static unsigned int calculateEndingTime(Solution* solution, unsigned int r1, unsigned int r2);
    unsigned int routeReleaseDateRemoving(Solution* s, unsigned int r, unsigned int vertex);
    static unsigned int verifySolutionChangingRoutes(Solution* solution, unsigned int r1, unsigned int r2,
                                                     unsigned int r1RD, unsigned int r1Time, unsigned int r2RD,
                                                     unsigned int r2Time);

    void shuffleSearchOrder();

   public:
    explicit InterSearch(const Instance& instance);
    unsigned int search(Solution* solution);
};

#endif  // TSPRD_INTERSEARCH_H
