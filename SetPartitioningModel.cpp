#include "SetPartitioningModel.h"
#include <limits>
#include <string>

#pragma ide diagnostic ignored "EndlessLoop"

static const int INF = numeric_limits<int>::max();

void SetPartitioningModel::getA(vector<vector<int>> &a){
    unsigned int lenRoute;

    for(unsigned int i = 0; i < routesData.size(); i++){
        lenRoute = routesData[i]->route.size() - 1;
        for(unsigned int j = 1; j < lenRoute; j++){
            a[i][routesData[i]->route[j] - 1] = 1;
        }
    }
}

SetPartitioningModel::SetPartitioningModel(vector<RouteData *> &routesData, unsigned int nClients) : routesData(routesData), nClients(nClients) {
    this->nClients = nClients;

    vector<vector<int>> a(routesData.size(), vector<int>(nClients, 0));
    
    routes = addConstraints(a);
}

vector<vector<unsigned int>*> SetPartitioningModel::addConstraints(vector<vector<int>> a){
    IloEnv env;
    IloModel model(env);
    IloArray<IloBoolVarArray> x;
    IloIntVarArray T0;
    IloIntVarArray Tf;

    
    getA(a);

    unsigned int nRoutes = routesData.size();

    //Cria matriz
    x = IloArray< IloBoolVarArray > (env, nRoutes);
    for(unsigned int i = 0; i < nRoutes; i++) {
        IloBoolVarArray vetor_i(env, nClients);
        x[i] = vetor_i;
    }

    //Adiciona variáveis x ao modelo
    char var[100];
    for(unsigned int i = 0; i < nRoutes; i++) {
        for(unsigned int j = 0; j < nClients; j++) {
            x[i][j].setName(std::string("x" + std::to_string(i) + "_" + std::to_string(j)).c_str());
            //model.add(x[i][j]);
        }
    }

    try{
        T0 = IloIntVarArray(env, nClients, 0, INF);
        Tf = IloIntVarArray(env, nClients, 0, INF);


        //Adiciona variáveis T0 e Tf ao modelo
        for(unsigned int i = 0; i < nClients; i++){
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
        //cout << Tf << endl;

        model.add(IloMinimize(env, Tf[nClients - 1])); // FO
    }catch(IloException& e){
        cerr << "CPLEX found the following exception: " << e << endl;
        e.end();
    }catch(...){
        cerr << "The following unknown exception was found: " << endl;
    } 

    char c_name[32];

    for(unsigned int j = 0; j < nClients; j++){ // (1)
        IloExpr sum(env);   
        for(unsigned int k = 0; k < nClients; k++){
            for(unsigned int i = 0; i < nRoutes; i++){
                sum += (int) a[i][j] * x[i][k];
            }
        }

        IloConstraint c1 = (sum == 1);

        sprintf(c_name, "c_1(%d)", j);
        c1.setName((char*) c_name);

        model.add(c1);
        sum.end();
    }

    for (unsigned int k = 0; k < nClients; k++) { // (2)
        IloExpr sum1(env);
        for (unsigned int i = 0; i < nRoutes; i++) {
            sum1 += x[i][k];
        }

        IloConstraint c2 = (sum1 <= 1);

        sprintf(c_name, "c_2(%d)", k);
        c2.setName((char*) c_name);

        model.add(c2);
        sum1.end();
    }


    for (unsigned int k = 0; k < nClients; k++) { // (3)
        IloExpr sum2(env);
        for (unsigned int i = 0; i < nRoutes; i++) {
            sum2 += (int) routesData[i]->releaseTime * x[i][k];
        }

        IloConstraint c3 = (T0[k] >= sum2);

        sprintf(c_name, "c_3(%d)", k);
        c3.setName((char*) c_name);

        model.add(c3);
        sum2.end();
    }

    for (unsigned int k = 1; k < nClients; k++) { // (4)
        IloConstraint c4 = (T0[k] >= Tf[k-1]);
        
        sprintf(c_name, "c_4(%d)", k);
        c4.setName((char*) c_name);

        model.add(c4);
    }

    for (unsigned int k = 0; k < nClients; k++) { // (5)
        IloExpr sum3(env);
        for (unsigned int i = 0; i < nRoutes; i++) {
            sum3 += (int) routesData[i]->duration * x[i][k];
        }
        IloConstraint c5 = (Tf[k] >= T0[k] + sum3);

        sprintf(c_name, "c_5(%d)", k);
        c5.setName((char*) c_name);

        model.add(c5);
        sum3.end();
    }

    for (unsigned int k = 0; k < nClients-1; k++) { // (6)
        IloExpr sum4(env);
        IloExpr sum5(env);

        for (unsigned int i = 0; i < nRoutes; i++) {
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
    cplex.setParam(IloCplex::Threads, 1);
    
    //cout << this->model << endl;

    try{
        //cout << "extrair" << endl;
        cplex.extract(model);

        //cplex.exportModel("output/model.lp");
        //cout << "resolver o modelo" << endl;

        cplex.solve();

    }catch(IloException& e){
        cerr << "CPLEX found the following exception: " << e << endl;
        e.end();
    }catch(...){
        cerr << "The following unknown exception was found: " << endl;
    }
    

    //cout << "------------- Results -------------------" << endl;
    //cout << cplex.getCplexStatus() << endl;
    //cout << "Result: " << cplex.getObjValue() << endl << endl;
    //cout << "Time: " << cplex.getTime() << endl;

    this->time = cplex.getTime();

    vector<vector<unsigned int>*> routes;

    for(unsigned int k = 0; k < nClients; k++){
        for(unsigned int i = 0; i < nRoutes; i++){
            if(cplex.getValue(x[i][k]) == true){
                routes.push_back(&routesData[i]->route);
            }
        }
    }

    env.end();

    return routes;
}

IloNum SetPartitioningModel::getTime(){
    return this->time;
}