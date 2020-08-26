#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H


#include "Instance.h"
#include "Solution.h"

class NeighborSearch {
private:
    const Instance& instance;
    int callIntraSearch(vector<unsigned int> &route, int which);
    int verifySwap(vector<unsigned int> &route, int i1, int i2, int n1, int n2);
    int swapSearchIt(vector<unsigned int> &route, int n1, int n2);
    int reinsertionSearchIt(vector<unsigned int> &route, int n);
    int twoOptSearchIt(vector<unsigned int> &route);
public:
    explicit NeighborSearch(const Instance& instance);
    int intraSearch(Solution *solution);
    int swapSearch(vector<unsigned int> &route, int n1 = 1, int n2 = 1);
    int reinsertionSearch(vector<unsigned int> &route, int n = 1);
    int twoOptSearch(vector<unsigned int> &route);
    int educate(Solution *solution);
};


#endif //TSPRD_NEIGHBORSEARCH_H
