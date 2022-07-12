#ifndef TSPRD_INTRASEARCH_H
#define TSPRD_INTRASEARCH_H

#include "Data.h"
#include "Solution.h"

class IntraSearch {
   private:
    Data& data;
    std::vector<int> searchOrder;

    void shuffleSearchOrder();
    int callIntraSearch(std::vector<int>* route, int which);
    int swapSearch(std::vector<int>* route, int n1 = 1, int n2 = 1);
    int evaluateSwap(std::vector<int>* route, int i1, int i2, int n1, int n2);
    int reinsertionSearch(std::vector<int>* route, int n = 1);
    int twoOptSearch(std::vector<int>* route);

   public:
    explicit IntraSearch(Data& Data);
    int search(Solution* solution);
};

#endif  // TSPRD_INTRASEARCH_H
