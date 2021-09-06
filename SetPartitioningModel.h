#ifndef TSPRD_SETPARTITIONINGMODEL_H
#define TSPRD_SETPARTITIONINGMODEL_H

#include <ilcplex/ilocplex.h>
#include <set>

#include "RoutePool.h"

class SetPartitioningModel {
public:

    SetPartitioningModel(vector<RouteData *> &routesData, unsigned int nClients, unsigned int bestSolutionHeuristic);

    vector<vector<unsigned int>*> routes;
    IloAlgorithm::Status getstatus();
    IloCplex::CplexStatus getcplexStatus();
    IloNum getobjValue();
    IloNum getbestObjValue();
    IloNum getTime();
    IloNum getcplexTime();
    IloInt getNnodes();
    IloNum getcutoff();

private:
    vector<RouteData *> &routesData;
    unsigned int nClients;

    IloAlgorithm::Status status;
    IloCplex::CplexStatus cplexStatus;
    IloNum objValue;
    IloNum bestObjValue;
    IloNum time;
    IloNum cplexTime;
    IloInt Nnodes;
    IloNum cutoff;






    vector<vector<unsigned int>*> addConstraints(vector<vector<int>> a, unsigned int bestSolutionHeuristic);
    void getA(vector<vector<int>> &a);
};


#endif //TSPRD_SETPARTITIONINGMODEL_H
