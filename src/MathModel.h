#ifndef TSPRD_MATHMODEL_H
#define TSPRD_MATHMODEL_H

#include <ilcplex/ilocplex.h>

#include <set>

// this math model is defined by archetti
class MathModel {
   public:
    explicit MathModel(const Instance& instance);
    // uses the math model to perform a split on the sequence
    explicit MathModel(const Instance& instance, const vector<unsigned int>& sequence);
    MathModel(const Instance& instance, vector<set<unsigned int> >& adjList);

   private:
    const Instance& instance;
    vector<set<unsigned int> > adjList;  // arcs to consider
    unsigned int modV;                   // size of the vertex set
    unsigned int modK;                   // size of the routes set

    IloEnv env;
    IloModel model;
    IloArray<IloArray<IloBoolVarArray> > x;
    IloArray<IloBoolVarArray> y;
    IloArray<IloArray<IloIntVarArray> > u;
    IloIntVarArray Ts;
    IloIntVarArray Te;

    void addConstraints();
    void execute();
};

#endif  // TSPRD_MATHMODEL_H
