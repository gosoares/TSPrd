#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H


#include "Instance.h"
#include "Solution.h"

class NeighborSearch {
private:
    const Instance& instance;
    const vector<vector<unsigned int> > &W;
    const vector<unsigned int> &RD;

    unsigned int callIntraSearch(vector<unsigned int> &route, unsigned int which);
    unsigned int swapSearch(vector<unsigned int> &route, unsigned int n1 = 1, unsigned int n2 = 1);
    unsigned int swapSearchIt(vector<unsigned int> &route, unsigned int n1, unsigned int n2);
    int verifySwap(vector<unsigned int> &route, unsigned int i1, unsigned int i2,
                            unsigned int n1, unsigned int n2);
    unsigned int reinsertionSearch(vector<unsigned int> &route, unsigned int n = 1);
    unsigned int reinsertionSearchIt(vector<unsigned int> &route, unsigned int n);
    unsigned int twoOptSearch(vector<unsigned int> &route);
    unsigned int twoOptSearchIt(vector<unsigned int> &route);

    unsigned int callInterSearch(Solution *solution, int which);
    static unsigned int calculateEndingTime(Solution *solution, unsigned int r1, unsigned int r2);
    unsigned int routeReleaseDateRemoving(Solution *s, unsigned int r, unsigned int vertex);
    static unsigned int verifySolutionChangingRoutes(
            Solution *solution, unsigned int r1, unsigned int r2,
            unsigned int r1RD, unsigned int r1Time, unsigned int r2RD, unsigned int r2Time
    );
    unsigned int vertexRelocation(Solution *solution);
    unsigned int vertexRelocationIt(Solution *solution, unsigned int r1, unsigned int r2);
    unsigned int interSwap(Solution *solution);
    unsigned int interSwapIt(Solution *solution, unsigned int r1, unsigned int r2);
public:
    explicit NeighborSearch(const Instance& instance);
    unsigned int intraSearch(Solution *solution);
    unsigned int interSearch(Solution *solution);
    unsigned int educate(Solution *solution);
};


#endif //TSPRD_NEIGHBORSEARCH_H
