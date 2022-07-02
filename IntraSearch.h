#ifndef TSPRD_INTRASEARCH_H
#define TSPRD_INTRASEARCH_H

#include <random>
#include "Instance.h"
#include "Solution.h"

class IntraSearch {
private:
    const vector<vector<unsigned int> > &W;

    vector<unsigned int> searchOrder;
    mt19937 generator;

    void shuffleSearchOrder();
    unsigned int callIntraSearch(vector<unsigned int> *route, unsigned int which);
    unsigned int swapSearch(vector<unsigned int> *route, unsigned int n1 = 1, unsigned int n2 = 1);
    int evaluateSwap(vector<unsigned int> *route, unsigned int i1, unsigned int i2,
                     unsigned int n1, unsigned int n2);
    unsigned int reinsertionSearch(vector<unsigned int> *route, unsigned int n = 1);
    unsigned int twoOptSearch(vector<unsigned int> *route);

public:
    explicit IntraSearch(const Instance &instance);
    unsigned int search(Solution *solution);
};

#endif //TSPRD_INTRASEARCH_H
