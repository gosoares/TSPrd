#include "MathModel.h"

#pragma ide diagnostic ignored "EndlessLoop"

MathModel::MathModel(
        const Instance &instance
) : instance(instance), adjList(instance.nVertex()), modV(instance.nVertex()), modK(instance.nVertex() - 1), model(env),
    x(env, instance.nVertex()), y(env, instance.nVertex()), u(env, instance.nVertex()),
    Ts(env, instance.nVertex(), 0, IloInfinity), Te(env, instance.nVertex(), 0, IloInfinity) {
    for (int i = 0; i < instance.nVertex(); i++) {
        for (int j = 0; j < instance.nVertex(); j++) {
            if (i == j) continue;
            adjList[i].insert(j);
        }
    }
    addConstraints();
    execute();
}

MathModel::MathModel(
        const Instance &instance, const vector<unsigned int> &sequence
) : instance(instance), adjList(instance.nVertex()), modV(instance.nVertex()), modK(instance.nVertex() - 1), model(env), x(env, instance.nVertex()),
    y(env, instance.nVertex()), u(env, instance.nVertex()), Ts(env, instance.nVertex(), 0, IloInfinity),
    Te(env, instance.nVertex(), 0, IloInfinity) {
    for (unsigned int i = 1; i < sequence.size(); i++) {
        adjList[0].insert(i);
        adjList[i].insert(0);
        adjList[sequence[i - 1]].insert(sequence[i]);
    }
    addConstraints();
    vector<IloExpr> routeIndex(instance.nVertex());
    for (int i = 1; i < modV; i++) { // calcula indice da rota de cada cliente
        routeIndex[i] = IloExpr(env);
        for (int k = 0; k < modK; k++) {
            routeIndex[i] += k * y[i][k];
        }
    }
    for (int i = 1; i < sequence.size(); i++) { // restricao para cada par de cliente consecutivo na sequencia
        model.add(routeIndex[sequence[i]] >= routeIndex[sequence[i - 1]]);
    }
    execute();
}

MathModel::MathModel(
        const Instance &instance, vector<set<unsigned int> > &adjList
) : instance(instance), adjList(adjList), modV(instance.nVertex()), modK(instance.nVertex() - 1), model(env),
    x(env, instance.nVertex()), y(env, instance.nVertex()), u(env, instance.nVertex()),
    Ts(env, instance.nVertex(), 0, IloInfinity), Te(env, instance.nVertex(), 0, IloInfinity) {
    addConstraints();
    execute();
}

void MathModel::addConstraints() {
    char var[100];
    for (int i = 0; i < modV; i++) { // criando variaveis x
        IloArray<IloBoolVarArray> matrix(env, modV);
        x[i] = matrix;
        for (int j = 0; j < modV; j++) {
            IloBoolVarArray array(env, modK);
            x[i][j] = array;

            if (adjList[i].find(j) != adjList[i].end()) { // se existir o arco
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
        u[i] = IloArray<IloIntVarArray>(env, modV);
        for (int j = 0; j < modV; j++) {
            u[i][j] = IloIntVarArray(env, modK, 0, IloInfinity);

            if (adjList[i].find(j) != adjList[i].end()) { // se existir o arco
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

    for (int i = 0; i < modV; i++) { // (3)
        for (int k = 0; k < modK; k++) {
            IloExpr sumXijk(env);
            IloExpr sumXjik(env);


            for (auto &j: adjList[i]) {
                auto &a = x[i];
                auto &b = a[j];
                auto &c = b[k];
                sumXijk += c;
            }
            for (int j = 0; j < adjList.size(); j++) {
                if (adjList[j].find(i) != adjList[j].end()) {
                    sumXjik += x[j][i][k];
                }
            }

            model.add(sumXjik == sumXijk);
            model.add(sumXijk == y[i][k]);
        }
    }

    for (int i = 1; i < modV; i++) { // (4)
        for (int k = 0; k < modK; k++) {
            IloExpr sumUjik(env);
            IloExpr sumUijk(env);

            for (auto &j: adjList[i]) {
                sumUijk += u[i][j][k];
            }
            for (int j = 0; j < adjList.size(); j++) {
                if (adjList[j].find(i) != adjList[j].end()) {
                    sumUjik += u[j][i][k];
                }
            }

            model.add(sumUjik - sumUijk == y[i][k]);
        }
    }

    for (int i = 0; i < adjList.size(); i++) {
        for (int j : adjList[i]) {
            for (int k = 0; k < modK; k++) {
                model.add(u[i][j][k] <= (int) (modV - 1) * x[i][j][k]);
            }
        }
    }

    for (int k = 0; k < modK; k++) { // (6)
        IloExpr sumTimes(env);
        for (int i = 0; i < adjList.size(); i++) {
            for (int j : adjList[i]) {
                sumTimes += (int) instance.time(i, j) * x[i][j][k];
            }
        }

        model.add(Te[k] == Ts[k] + sumTimes);
    }

    for (int k = 0; k < modK - 1; k++) { // (7)
        model.add(Te[k] <= Ts[k + 1]);
    }

    for (int k = 0; k < modK; k++) { // (8)
        for (int i = 1; i < modV; i++) {
            model.add(Ts[k] >= (int) instance.releaseTimeOf(i) * y[i][k]);
        }
    }

    for (int k = 0; k < modK - 1; k++) { // (9)
        model.add(Te[k] == Ts[k + 1]);
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

    for (int i = 0; i < adjList.size(); i++) {
        for (int j : adjList[i]) {
            for (int k = 0; k < modK - 1; k++) {
                model.add(x[i][j][k] <= 1 - x[0][0][k]);
            }
        }
    }

    for (int k = 0; k < modK - 1; k++) { // (12)
        model.add(x[0][0][k] >= x[0][0][k + 1]);
    }
}

void MathModel::execute() {
    IloCplex cplex(model);

    cplex.exportModel("output/model.lp");
    cplex.solve();

    cout << cplex.getCplexStatus() << endl;
    cout << "Result: " << cplex.getValue(Te[modK - 1]) << endl << endl;

    cout << "Empty routes: ";
    for (int k = 0; k < modK; k++) {
        if (cplex.getValue(x[0][0][k]) > 0.99) {
            cout << k << "  ";
            continue;
        } else {
            cout << endl << endl << "Route " << k << ": " << endl;
            cout << "T: " << cplex.getValue(Ts[k]) << "~" << cplex.getValue(Te[k]) << endl;
        }

        for (int i = 0; i < adjList.size(); i++) {
            for (int j : adjList[i]) {

                if (cplex.getValue(x[i][j][k]) > 0.99) {
                    cout << i << "->" << j << "  ";
                }
            }
        }
    }
    cout << endl;

//    cout << "Modelo:  ";
//    for (int k = 0; k < modK; k++) {
//        for (int i = 1; i < modV; i++) {
//            if (i != sequence.back() && cplex.getValue(x[i][0][k]) > 0.99) {
//                cout << i << "  ";
//            }
//        }
//    }
    cout << endl;
    cout << "Result: " << cplex.getValue(Te[modK - 1]) << endl << endl;
}