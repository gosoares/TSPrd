//
// Created by gabriel on 7/6/20.
//

#ifndef TSPRD_NEIGHBORSEARCH_H
#define TSPRD_NEIGHBORSEARCH_H


#include "Instance.h"

class NeighborSearch {
private:
    const Instance& instance;
    int verifySwap(vector<unsigned int> &route, int i1, int i2, int n1, int n2);
    int swapSearchIt(vector<unsigned int> &route, int n1, int n2);
    int reinsertionSearchIt(vector<unsigned int> &route, int n);
    int twoOptSearchIt(vector<unsigned int> &route);
public:
    explicit NeighborSearch(const Instance& instance);
    int swapSearch(vector<unsigned int> &route, int n1 = 1, int n2 = 1);
    int reinsertionSearch(vector<unsigned int> &route, int n = 1);
    int twoOptSearch(vector<unsigned int> &route);
};


#endif //TSPRD_NEIGHBORSEARCH_H
