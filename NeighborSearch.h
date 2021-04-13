#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H


#include "Instance.h"
#include "Solution.h"

class NeighborSearch {
private:
    const Instance& instance;
    const vector<vector<unsigned int> > &W;
    const vector<unsigned int> &RD;

    int callIntraSearch(vector<unsigned int> &route, int which);
    int verifySwap(vector<unsigned int> &route, int i1, int i2, int n1, int n2);
    int swapSearchIt(vector<unsigned int> &route, int n1, int n2);
    int reinsertionSearchIt(vector<unsigned int> &route, int n);
    int twoOptSearchIt(vector<unsigned int> &route);

    static unsigned int calculateEndingTime(Solution *solution, unsigned int r1, unsigned int r2);
    unsigned int routeReleaseDateRemoving(Solution *s, unsigned int r, unsigned int vertex);
    static unsigned int verifySolutionChangingRoutes(
            Solution *solution, unsigned int r1, unsigned int r2,
            unsigned int r1RD, unsigned int r1Time, unsigned int r2RD, unsigned int r2Time
    );
    unsigned int vertexRelocation(Solution *solution);
    unsigned int vertexRelocationIt(Solution *solution, unsigned int r1, unsigned int r2);
    unsigned int interSwapIt(Solution *solution, unsigned int r1, unsigned int r2);
public:
    explicit NeighborSearch(const Instance& instance);
    int intraSearch(Solution *solution);
    int swapSearch(vector<unsigned int> &route, int n1 = 1, int n2 = 1);
    int reinsertionSearch(vector<unsigned int> &route, int n = 1);
    int twoOptSearch(vector<unsigned int> &route);
    int educate(Solution *solution);
};


#endif //TSPRD_NEIGHBORSEARCH_H
