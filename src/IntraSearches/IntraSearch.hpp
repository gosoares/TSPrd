#ifndef TSPRD_INTRASEARCH_H
#define TSPRD_INTRASEARCH_H

#include <vector>

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route

class IntraSearch {
   public:
    virtual int search(std::vector<int>* route) = 0;
};

#endif  // TSPRD_INTRASEARCH_H
