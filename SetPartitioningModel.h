#ifndef TSPRD_SETPARTITIONINGMODEL_H
#define TSPRD_SETPARTITIONINGMODEL_H

#include <ilcplex/ilocplex.h>
#include <set>

#include "RoutePool.h"

class SetPartitioningModel {
public:

    SetPartitioningModel(vector<RouteData *> &routesData, unsigned int nClients);
    IloNum getTime();

    vector<vector<unsigned int>*> routes;

private:
    vector<RouteData *> &routesData;
    unsigned int nClients;

    IloNum time;

    vector<vector<unsigned int>*> addConstraints(vector<vector<int>> a);
    void getA(vector<vector<int>> &a);
};


#endif //TSPRD_SETPARTITIONINGMODEL_H
