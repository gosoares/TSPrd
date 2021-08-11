#include "MathModelRoutes.h"
#include <limits>
#include <string>

#pragma ide diagnostic ignored "EndlessLoop"

static const int INF = numeric_limits<int>::max();
/*
unsigned int calcModK(const Instance &instance) {
//    double x = 0;
//    unsigned int biggerRD = 0;
//    for (int i = 1; i < instance.nVertex(); i++) {
//        x += instance.time(0, i) + instance.time(i, 0);
//        biggerRD = max(biggerRD, instance.releaseDateOf(i));
//    }
//    x = (biggerRD * (instance.nClients() - 1)) / x;
//    unsigned int modK = 1 + (unsigned int) floor(x);
//    cout << "modK: " << modK << endl;
//    return modK;
    return instance.nClients();
}

vector<unsigned int> a



MathModelRoutes::MathModelRoutes(
        const Instance &instance
) : instance(instance), adjList(instance.nVertex()), modV(instance.nVertex()), modK(calcModK(instance)), model(env),
    x(env, instance.nVertex()), y(env, instance.nVertex()), u(env, instance.nVertex()),
    Ts(env, instance.nVertex(), 0, INF), Te(env, instance.nVertex(), 0, INF) {
    for (int i = 0; i < instance.nVertex(); i++) {
        for (int j = 0; j < instance.nVertex(); j++) {
            if (i == j) continue;
            adjList[i].insert(j);
        }
    }

    addConstraints();
    execute();
}

MathModelRoutes::MathModelRoutes(
        const Instance &instance, const vector<unsigned int> &sequence
) : instance(instance), adjList(instance.nVertex()), modV(instance.nVertex()), modK(calcModK(instance)), model(env), x(env, instance.nVertex()),
    y(env, instance.nVertex()), u(env, instance.nVertex()), Ts(env, instance.nVertex(), 0, INF),
    Te(env, instance.nVertex(), 0, INF) {
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

MathModelRoutes::MathModelRoutes(
        const Instance &instance, vector<set<unsigned int> > &adjList
) : instance(instance), adjList(adjList), modV(instance.nVertex()), modK(calcModK(instance)), model(env),
    x(env, instance.nVertex()), y(env, instance.nVertex()), u(env, instance.nVertex()),
    Ts(env, instance.nVertex(), 0, INF), Te(env, instance.nVertex(), 0, INF) {
    addConstraints();
    execute();
}*/

void MathModelRoutes::getA(vector<vector<unsigned int>> &a){
    int lenRoute = 0;

    for(int i = 0; i < nRoutes; i++){
        lenRoute = routePool.routes[i]->route.size();
        for(int j = 0; j < lenRoute; j++){
            a[i][j] = 1;
        }
    }
}

MathModelRoutes::MathModelRoutes(RoutePool &routePool, unsigned int nRoutes, unsigned int nClients, vector<vector<unsigned int>*> &routes) : routePool(routePool){
    //this->routePool = routePool;
    this->nRoutes = nRoutes;
    this->nClients = nClients;

    vector<vector<unsigned int>> a;
    for(int i = 0; i < nRoutes; i++){
        vector<unsigned int> vectorA(nClients, 0);
        a.push_back(vectorA);
    }


    
    routes = addConstraints(a);
}

vector<vector<unsigned int>*> MathModelRoutes::addConstraints(vector<vector<unsigned int>> a){
    IloEnv env;
    IloModel model(env);
    IloArray<IloBoolVarArray> x;
    IloIntVarArray T0;
    IloIntVarArray Tf;

    
    getA(a);

    //Cria matriz
    x = IloArray< IloBoolVarArray > (env, nRoutes);
    for(int i = 0; i < nRoutes; i++) {
        IloBoolVarArray vetor_i(env, nClients);
        x[i] = vetor_i;
    }

    //Adiciona variáveis x ao modelo
    char var[100];
    for(int i = 0; i < nRoutes; i++) {
        for(int j = 0; j < nClients; j++) {
            x[i][j].setName(std::string("x" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            //model.add(x[i][j]);
        }
    }

    try{
        T0 = IloIntVarArray(env, nClients, 0, INF);
        Tf = IloIntVarArray(env, nClients, 0, INF);


        //Adiciona variáveis T0 e Tf ao modelo
        for(int i = 0; i < nClients; i++){
            T0[i].setName(std::string("T0_" + std::to_string(i)).c_str());

            Tf[i].setName(std::string("Tf_" + std::to_string(i)).c_str());
        }

    }catch(IloException& e){
        cerr << "CPLEX found the following exception: " << e << endl;
        e.end();
    }catch(...){
        cerr << "The following unknown exception was found: " << endl;
    }   


    //------------------- Modelo ----------------------------//
    try{
        cout << Tf << endl;

        model.add(IloMinimize(env, Tf[nClients - 1])); // FO
    }catch(IloException& e){
        cerr << "CPLEX found the following exception: " << e << endl;
        e.end();
    }catch(...){
        cerr << "The following unknown exception was found: " << endl;
    } 

    char c_name[32];

    for(int j = 0; j < nClients; j++){ // (1)
        IloExpr sum(env);   
        for(int k = 0; k < nClients; k++){
            for(int i = 0; i < nRoutes; i++){
                sum += (int) a[i][j] * x[i][k];
            }
        }

        IloConstraint c1 = (sum == 1);

        sprintf(c_name, "c_1(%d)", j);
        c1.setName((char*) c_name);

        model.add(c1);
        sum.end();
    }

    for (int k = 0; k < nClients; k++) { // (2)
        IloExpr sum1(env);
        for (int i = 0; i < nRoutes; i++) {
            sum1 += x[i][k];
        }

        IloConstraint c2 = (sum1 <= 1);

        sprintf(c_name, "c_2(%d)", k);
        c2.setName((char*) c_name);

        model.add(c2);
        sum1.end();
    }


    for (int k = 0; k < nClients; k++) { // (3)
        IloExpr sum2(env);
        for (int i = 0; i < nRoutes; i++) {
            sum2 += (int) routePool.routes[i]->releaseTime * x[i][k];
        }

        IloConstraint c3 = (T0[k] >= sum2);

        sprintf(c_name, "c_3(%d)", k);
        c3.setName((char*) c_name);

        model.add(c3);
        sum2.end();
    }

    for (int k = 1; k < nClients; k++) { // (4)
        IloConstraint c4 = (T0[k] >= Tf[k-1]);
        
        sprintf(c_name, "c_4(%d)", k);
        c4.setName((char*) c_name);

        model.add(c4);
    }

    for (int k = 0; k < nClients; k++) { // (5)
        IloExpr sum3(env);
        for (int i = 0; i < nRoutes; i++) {
            sum3 += (int) routePool.routes[i]->duration * x[i][k];
        }
        IloConstraint c5 = (Tf[k] >= T0[k] + sum3);

        sprintf(c_name, "c_5(%d)", k);
        c5.setName((char*) c_name);

        model.add(c5);
        sum3.end();
    }

    for (int k = 0; k < nClients-1; k++) { // (6)
        IloExpr sum4(env);
        IloExpr sum5(env);

        for (int i = 0; i < nRoutes; i++) {
            sum4 += x[i][k];
            sum5 += x[i][k+1];
        }
        IloConstraint c6 = (sum4 >= sum5);

        sprintf(c_name, "c_6(%d)", k);
        c6.setName((char*) c_name);

        model.add(c6);
        sum4.end();
        sum5.end();
    }

    // ------------------ Resolução do modelo ------------------//

    IloCplex cplex(env);
    //cout << this->model << endl;

    try{
        cout << "extrair" << endl;
        cplex.extract(model);

        //cplex.exportModel("output/model.lp");
        cout << "resolver o modelo" << endl;

        cplex.solve();

    }catch(IloException& e){
        cerr << "CPLEX found the following exception: " << e << endl;
        e.end();
    }catch(...){
        cerr << "The following unknown exception was found: " << endl;
    }
    

    cout << "------------- Results -------------------" << endl;
    cout << cplex.getCplexStatus() << endl;
    cout << "Result: " << cplex.getObjValue() << endl << endl;

    vector<vector<unsigned int>*> routes;

    for(int k = 0; k < nClients; k++){
        for(int i = 0; i < nRoutes; i++){
            if(cplex.getValue(x[i][k]) == true){
                routes.push_back(&routePool.routes[i]->route);
            }
        }
    }

    return routes;
}

/*
void MathModelRoutes::addConstraints() {
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
            u[i][j] = IloIntVarArray(env, modK, 0, INF);

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
            model.add(Ts[k] >= (int) instance.releaseDateOf(i) * y[i][k]);
        }
    }

    for (int k = 0; k < modK - 1; k++) { // (9)
        model.add(Te[k] == Ts[k + 1]);
    }

    int maxReleaseDate = 0;
    for (int i = 1; i < modV; i++) {
        int rdi = (int) instance.releaseDateOf(i);
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

void MathModelRoutes::execute() {
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
        }8////////.lllllllll9

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
}*/