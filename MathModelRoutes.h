#ifndef TSPRD_MATHMODELROUTES_H
#define TSPRD_MATHMODELROUTES_H

#include <ilcplex/ilocplex.h>
#include <set>

#include "RoutePool.h"

// this math model is defined by archetti
class MathModelRoutes {
public:
    //explicit MathModelRoutes(const Instance &instance);
    // uses the math model to perform a split on the sequence
    //explicit MathModelRoutes(const Instance &instance, const vector<unsigned int> &sequence);
    //MathModelRoutes(const Instance &instance, vector<set<unsigned int> > &adjList);
    MathModelRoutes(RoutePool &routePool, unsigned int nRoutes, unsigned int nClients, vector<vector<unsigned int>*> &routes);
    IloNum getTime();


private:
    RoutePool &routePool;

    unsigned int nRoutes;
    unsigned int nClients;

    /*
    vector<set<unsigned int> > adjList; // arcs to consider
    unsigned int modV; // size of the vertex set
    unsigned int modK; // size of the routes set
    */

    IloNum time;
    
    

    vector<vector<unsigned int>*> addConstraints(vector<vector<int>> a);
    void getA(vector<vector<int>> &a);
};


#endif //TSPRD_MATHMODELROUTES_H
