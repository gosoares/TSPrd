#ifndef TSPRD_MATHMODEL_H
#define TSPRD_MATHMODEL_H

#include <ilcplex/ilocplex.h>

#include "Instance.h"

class MathModel {
public:
    MathModel(const Instance& instance, vector<unsigned int> sequence);
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


#endif //TSPRD_MATHMODEL_H
