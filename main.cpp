#include <iostream>
#include <fstream>
#include "Solution.h"
#include "GeneticAlgorithm.h"

using namespace std;

unsigned int normalizeTime(unsigned int time) {
    return (unsigned int) (time * (1201.0 / 1976.0));
}

int main(int argc, char **argv) {
    // genetic algorithm parameters
    unsigned int mi = 20;
    unsigned int lambda = 40;
    auto nbElite = (unsigned int) (0.5 * mi);
    auto nClose = (unsigned int) (0.3 * mi);
    unsigned int itNi = 10000; // max iterations without improvement to stop the algorithm
    auto itDiv = (unsigned int) (0.4 * itNi); // iterations without improvement to diversify

    auto timeLimit = (unsigned int) (10 * 60 * (1976.0 / 1201.0)); // in seconds

    string instanceName = argv[1];
    Instance instance(instanceName);

    auto alg = GeneticAlgorithm(instance, mi, lambda, nClose, nbElite, itNi, itDiv, timeLimit);
    Solution s = alg.getSolution();
    s.validate();

    cout << "RESULT " << s.time << endl;
    cout << "EXEC_TIME " << alg.getExecutionTime() << endl;
    cout << "SOL_TIME " << alg.getBestSolutionTime() << endl;

    if (argc < 3) return 0;

    // output to file
    string outFile = string(argv[2]);
    string dir = outFile.substr(0, outFile.find_last_of('/'));
    if (dir != outFile) system(("mkdir -p " + dir).c_str()); // make sure the path exists

    ofstream fout(outFile, ios::out);
    fout << "EXEC_TIME " << normalizeTime(alg.getExecutionTime()) << endl;
    fout << "SOL_TIME " << normalizeTime(alg.getBestSolutionTime()) << endl;
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
    fout.close();

    // output search progress
    auto dotPos = outFile.find_last_of('.');
    string spFile = outFile.substr(0, dotPos) + "_SP" + outFile.substr(dotPos);
    ofstream spout(spFile, ios::out);
    spout << "time,obj" << endl;
    for (auto x: alg.getSearchProgress()) {
        spout << normalizeTime(x.first) << "," << x.second << endl;
    }
    spout.close();
    return 0;
}
