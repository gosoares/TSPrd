#ifndef TSPRD_SPLITMATHMODEL_H
#define TSPRD_SPLITMATHMODEL_H

#include <ilcplex/ilocplex.h>

#include "Instance.h"

class SplitMathModel {
public:
    SplitMathModel(const Instance& instance, const vector<unsigned int>& sequence);
private:
    const Instance& instance;
    IloEnv env;
    IloModel model;
    IloArray<IloArray<IloBoolVarArray> > x;
    IloArray<IloBoolVarArray> y;
    IloArray<IloArray<IloIntVarArray> > u;
    IloIntVarArray Ts;
    IloIntVarArray Te;
};


#endif //TSPRD_SPLITMATHMODEL_H
