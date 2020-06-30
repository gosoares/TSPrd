//
// Created by gabriel on 6/24/20.
//

#include "SplitMathModel.h"
#include<set>

SplitMathModel::SplitMathModel(const Instance &instance, vector<unsigned int> sequence) : instance(instance), model(env),
                                                                                          x(env, instance.nVertex()),
                                                                                          y(env, instance.nVertex()),
                                                                                          u(env, instance.nVertex()),
                                                                                          Ts(env, instance.nVertex(), 0, IloInfinity),
                                                                                          Te(env, instance.nVertex(), 0, IloInfinity) {

    unsigned int modV = instance.nVertex(), modK = instance.nVertex() - 1;
    set<pair<unsigned int, unsigned int> > A;
    vector<int> before(modV, -1);
    vector<int> next(modV, -1);

    for (int i = 0; i < sequence.size(); i++) {

        A.emplace(0, sequence[i]);
        A.emplace(sequence[i], 0);
        if (i + 1 < sequence.size()) {
            A.emplace(sequence[i], sequence[i + 1]);
            next[sequence[i]] = sequence[i + 1];
        }
        if (i != 0) {
            before[sequence[i]] = sequence[i - 1];
        }
    }

    char var[100];
    for (int i = 0; i < modV; i++) { // criando variaveis x
        IloArray<IloBoolVarArray> matrix(env, modV);
        x[i] = matrix;
        for (int j = 0; j < modV; j++) {
            IloBoolVarArray array(env, modK);
            x[i][j] = array;

            if (A.find(pair<unsigned int, unsigned int>(i, j)) != A.end()) { // se existir o arco
                for (int k = 0; k < modK; k++) {
                    sprintf(var, "x(%d,%d,%d)", i, j, k);
                    x[i][j][k].setName(var);
                    model.add(x[i][j][k]);
                }
            }

        }
    }

    // cria variaveis x[0][0][k]
    for (int k = 0; k < modK; k++) {
        sprintf(var, "x(%d,%d,%d)", 0, 0, k);
        x[0][0][k].setName(var);
        model.add(x[0][0][k]);
    }

    for (int i = 0; i < modV; i++) { // criando variaveis y
        IloBoolVarArray array(env, modK);
        y[i] = array;

        for (int k = 0; k < modK; k++) {
            sprintf(var, "y(%d,%d)", i, k);
            y[i][k].setName(var);
            model.add(y[i][k]);
        }
    }

    for (int i = 0; i < modV; i++) { // criando variaveis u
        IloArray<IloIntVarArray> matrix(env, modV);
        u[i] = matrix;
        for (int j = 0; j < modV; j++) {
            IloIntVarArray array(env, modK, 0, IloInfinity);
            u[i][j] = array;

            if (A.find(pair<unsigned int, unsigned int>(i, j)) != A.end()) { // se existir o arco
                for (int k = 0; k < modK; k++) {
                    sprintf(var, "u(%d,%d,%d)", i, j, k);
                    u[i][j][k].setName(var);
                    model.add(u[i][j][k]);
                }
            }

        }
    }

    for (int k = 0; k < modK; k++) { // cria variaveis Ts e Te
        sprintf(var, "Ts(%d)", k);
        Ts[k].setName(var);
        model.add(Ts[k]);

        sprintf(var, "Te(%d)", k);
        Te[k].setName(var);
        model.add(Te[k]);
    }

    model.add(IloMinimize(env, Te[modK - 1])); // FO

    for (int i = 1; i < modV; i++) { // (2)
        IloExpr sum(env);
        for (int k = 0; k < modK; k++) {
            sum += y[i][k];
        }
        model.add(sum == 1);
    }

    for (int i = 1; i < modV; i++) { // (3)
        for (int k = 0; k < modK; k++) {
            IloExpr sumXijk(env);
            IloExpr sumXjik(env);

            sumXijk += x[i][0][k];
            sumXjik += x[0][i][k];
            if(next[i] != -1) {
                sumXijk += x[i][next[i]][k];
            }
            if (before[i] != -1) {
                sumXjik += x[before[i]][i][k];
            }

            model.add(sumXjik == sumXijk);
            model.add(sumXijk == y[i][k]);
        }
    }

    for (int k = 0; k < modK; k++) { // cont. (3)
        IloExpr sumX0jk(env);
        IloExpr sumXj0k(env);
        for (int j = 0; j < modV; j++) {
            sumX0jk += x[0][j][k];
            sumXj0k += x[j][0][k];
        }
        model.add(sumX0jk == sumXj0k);
        model.add(sumXj0k == y[0][k]);
    }

    for (int i = 1; i < modV; i++) { // (4)
        for (int k = 0; k < modK; k++) {
            IloExpr sumUjik(env);
            IloExpr sumUijk(env);

            sumUjik += u[0][i][k];
            if (before[i] != -1)
                sumUjik += u[before[i]][i][k];

            sumUijk += u[i][0][k];
            if(next[i] != -1)
                sumUijk += u[i][next[i]][k];

            model.add(sumUjik - sumUijk == y[i][k]);
        }
    }

    for(auto p: A) { // (5)
        unsigned int i = p.first;
        unsigned int j = p.second;

        for(int k = 0; k < modK; k++) {
            model.add(u[i][j][k] <= (int)(modV - 1) * x[i][j][k]);
        }
    }

    for (int k = 0; k < modK; k++) { // (6)
        IloExpr sumTimes(env);
        for(auto p: A) {
            unsigned int i = p.first;
            unsigned int j = p.second;

            sumTimes += (int) instance.time(i, j) * x[i][j][k];
        }

        model.add(Te[k] == Ts[k] + sumTimes);
    }

    for (int k = 0; k < modK - 1; k++) { // (7)
        model.add(Te[k] <= Ts[k+1]);
    }

    for( int k = 0; k < modK; k++) { // (8)
        for (int i = 1; i < modV; i++) {
            model.add(Ts[k] >= (int) instance.releaseTimeOf(i) * y[i][k]);
        }
    }

    for (int k = 0; k < modK - 1; k++) { // (9)
        model.add(Te[k] == Ts[k+1]);
    }

    int maxReleaseDate = 0;
    for (int i = 1; i < modV; i++) {
        int rdi = (int) instance.releaseTimeOf(i);
        if (rdi > maxReleaseDate) {
            maxReleaseDate = rdi;
        }
    }
    for (int k = 0; k < modK - 1; k++) { // (10)
        model.add(Ts[k] <= maxReleaseDate);
    }

    for(auto p: A) { // (11)
        unsigned int i = p.first;
        unsigned int j = p.second;

        for (int k = 0; k < modK - 1; k++) {
            model.add(x[i][j][k] <= 1 - x[0][0][k]);
        }

    }

    for (int k = 0; k < modK - 1; k++) { // (12)
        model.add(x[0][0][k] >= x[0][0][k+1]);
    }

    vector<IloExpr> routeIndex(modV);
    for(int i = 1; i < modV; i++) { // calcula indice da rota de cada cliente
        routeIndex[i] = IloExpr(env);
        for(int k = 0; k < modK; k++) {
            routeIndex[i] += k*y[i][k];
        }
    }
    for (int i = 1; i < sequence.size(); i++) { // restricao para cada par de cliente consecutivo na sequencia
        model.add(routeIndex[sequence[i]] >= routeIndex[sequence[i-1]]);
    }

    IloCplex cplex(model);

    cplex.exportModel("model.lp");
    cplex.solve();

    cout << cplex.getCplexStatus() << endl;
    cout << "Result: " << cplex.getValue(Te[modK - 1]) << endl << endl;

    cout << "Empty routes: ";
    for (int k = 0; k < modK; k++) {
        if(cplex.getValue(x[0][0][k]) > 0.99) {
            cout << k << "  ";
            continue;
        } else {
            cout << endl << endl << "Route " << k << ": " << endl;
            cout << "T: " << cplex.getValue(Ts[k]) << "~" << cplex.getValue(Te[k]) << endl;
        }

        for(auto p: A) {
            unsigned int i = p.first;
            unsigned int j = p.second;

            if (cplex.getValue(x[i][j][k]) > 0.99) {
                cout << i << "->" << j << "  ";
            }
        }
    }
    cout << endl;

    cout << "Modelo:  ";
    for (int k = 0; k < modK; k++) {
        for (int i = 1; i < modV; i++) {
            if(i != sequence.back() && cplex.getValue(x[i][0][k]) > 0.99) {
                cout << i << "  ";
            }
        }
    }
    cout << endl;
    cout << "Result: " << cplex.getValue(Te[modK - 1]) << endl << endl;

}
