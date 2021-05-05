#include <iostream>
#include <algorithm>
#include <fstream>
#include "Solution.h"
#include "GeneticAlgorithm.h"
#include "Grasp.h"

using namespace std;

int main(int argc, char **argv) {

    // genetic algorithm parameters
    unsigned int mi = 25;
    unsigned int lambda = 100;
    unsigned int nbElite = 10; // el = 0.4; nbElite = el * mi
    unsigned int nClose = 5;
    unsigned int itNi = 2000; // max iterations without improvement to stop the algorithm
    unsigned int itDiv = 100; // iterations without improvement to diversify

    // grasp parameters
    unsigned int itNiGrasp = 1000;
    double alpha = 0.2;

    unsigned int timeLimit = 10 * 60; // in seconds

    string instanceFile = argv[1];
    Instance instance(instanceFile);

    auto alg = GeneticAlgorithm(instance, mi, lambda, nClose, nbElite, itNi, itDiv, timeLimit);
//    auto alg = Grasp(instance, itNiGrasp, alpha, timeLimit);
    Solution s = alg.getSolution();
    s.validate();

    cout << "RESULT " << s.time << endl;
    cout << "EXEC_TIME " << alg.getExecutionTime() << endl;
    cout << "SOL_TIME " << alg.getBestSolutionTime() << endl;

    // output to file
    unsigned long long timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    string outFile = to_string(timeStamp) + "/" + instanceFile;
    if (argc > 2)
        outFile = string(argv[2]) + "/" + instanceFile;
    outFile = "output/" + outFile + ".txt";
    string dir = outFile.substr(0, outFile.find_last_of('/'));
    system(("mkdir -p " + dir).c_str());

    ofstream fout(outFile, ios::out);
    fout << "EXEC_TIME " << alg.getExecutionTime() << endl;
    fout << "SOL_TIME " << alg.getBestSolutionTime() << endl;
    fout << "OBJ " << s.time << endl;
    fout << "N_ROUTES " << s.routes.size() << endl;
    fout << "N_CLIENTS";
    for (auto &r: s.routes) fout << " " << (r.size() - 2);
    fout << endl << "ROUTES" << endl;
    for (auto &r: s.routes) {
        for (unsigned int c = 1; c < r.size() - 1; c++) {
            fout << r[c] << " ";
        }
        cout << endl;
    }
    fout << endl;
    fout.close();
    return 0;
}