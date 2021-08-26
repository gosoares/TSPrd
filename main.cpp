#include <iostream>
#include <algorithm>
#include <fstream>
#include "Solution.h"
#include "GeneticAlgorithm.h"
#include "Grasp.h"
#include "RoutePool.h"
#include "SetPartitioningModel.h"

using namespace std;

int main(int argc, char **argv) {

    // genetic algorithm parameters
    unsigned int mi = 20;
    unsigned int lambda = 40;
    auto nbElite = (unsigned int) (0.5 * mi);
    auto nClose = (unsigned int) (0.3 * mi);
    unsigned int itNi = 10000; // max iterations without improvement to stop the algorithm
    auto itDiv = (unsigned int) (0.4 * itNi); // iterations without improvement to diversify

    // grasp parameters
    unsigned int itNiGrasp = 1000;
    double alpha = 0.2;

    auto timeLimit = (unsigned int) (10 * 60 * (1976.0 / 1201.0)); // in seconds

    string instanceFile = argv[1];
    Instance instance(instanceFile);

    int sizePool = 10000;

    if (argc > 4)
        sizePool = atoi(argv[4]);

    RoutePool routePool(sizePool);

    auto alg = GeneticAlgorithm(instance, mi, lambda, nClose, nbElite, itNi, itDiv, timeLimit, routePool);
//    auto alg = Grasp(instance, itNiGrasp, alpha, timeLimit);
    Solution s = alg.getSolution();
    s.validate();

    vector<RouteData *> routes(routePool.getRoutes());
    SetPartitioningModel model(routes, instance.nClients());
    
    Solution sModel = Solution(instance, model.routes);

    cout << endl << endl;
    cout << "RESULT " << s.time << endl;
    cout << "EXEC_TIME " << alg.getExecutionTime() << endl;
    cout << "SOL_TIME " << alg.getBestSolutionTime() << endl;

    cout << "RESULT_MODEL " << sModel.time << endl;
    cout << "EXEC_TIME_MODEL " << model.getTime() << endl;
    cout << "COUNT_ROUTES " << routes.size() << endl;


    // output to file
    unsigned long long timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    string outFile = to_string(timeStamp) + "/" + instanceFile;
    if (argc > 2)
        outFile = string(argv[2]) + "/" + instanceFile;
    string id = "1";
    if (argc > 3)
        id = string(argv[3]);
    outFile = "output/" + outFile + "_" + id + ".txt";
    string dir = outFile.substr(0, outFile.find_last_of('/'));
    system(("mkdir -p " + dir).c_str());

    ofstream fout(outFile, ios::out);
    fout << "EXEC_TIME " << alg.getExecutionTime() << endl;
    fout << "SOL_TIME " << alg.getBestSolutionTime() << endl;
    fout << "OBJ " << s.time << endl;
    fout << "N_ROUTES " << s.routes.size() << endl;
    fout << "N_CLIENTS";
    for (auto &r: s.routes) fout << " " << (r->size() - 2);
    fout << endl << "ROUTES" << endl;
    for (auto &r: s.routes) {
        for (unsigned int c = 1; c < r->size() - 1; c++) {
            fout << r->at(c) << " ";
        }
        fout << endl;
    }
    fout << endl;

    fout << "EXEC_TIME " << model.getTime() << endl;
    //fout << "SOL_TIME " << alg.getBestSolutionTime() << endl;
    fout << "OBJ_MODEL " << sModel.time << endl;
    fout << "N_ROUTES_MODEL " << sModel.routes.size() << endl;
    fout << "N_CLIENTS_MODEL";
    for (auto &r: sModel.routes) fout << " " << (r->size() - 2);
    fout << endl << "ROUTES" << endl;
    for (auto &r: sModel.routes) {
        for (unsigned int c = 1; c < r->size() - 1; c++) {
            fout << r->at(c) << " ";
        }
        fout << endl;
    }
    fout << endl;
    fout.close();

    // output search progress
    string spFile = outFile.substr(0, outFile.find_last_of('.')) + "_SP.txt";
    ofstream spout(spFile, ios::out);
    for (auto x: alg.getSearchProgress()) {
        spout << x.first << "\t" << x.second << endl;
    }
    spout.close();
    return 0;
}