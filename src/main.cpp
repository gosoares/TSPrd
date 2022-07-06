#include <filesystem>
#include <fstream>
#include <iostream>

#include "GeneticAlgorithm.h"
#include "Rng.h"
#include "Solution.h"

using namespace std;

const double TIME_COEFF = 1201.0 / 1976.0;  // time normalization coefficient
long long Rng::seed;
mt19937* Rng::generator;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "usage: TSPrd instance_file [output_file] [seed]" << endl;
        exit(1);
    }

    // initialize Rng
    long long seed = argc == 4 ? stoll(argv[3]) : random_device{}();
    Rng::initialize(seed);

    // genetic algorithm parameters
    unsigned int mi = 20;
    unsigned int lambda = 40;
    auto nbElite = (unsigned int)(0.5 * mi);
    auto nClose = (unsigned int)(0.3 * mi);
    unsigned int itNi = 10000;                // max iterations without improvement to stop the algorithm
    auto itDiv = (unsigned int)(0.4 * itNi);  // iterations without improvement to diversify

    auto timeLimit = (unsigned int)(10 * 60 / TIME_COEFF);  // in seconds

    string instanceName = argv[1];
    Instance instance(instanceName);

    auto alg = GeneticAlgorithm(instance, mi, lambda, nClose, nbElite, itNi, itDiv, timeLimit);
    Solution s = alg.getSolution();
    s.validate();

    cout << "RESULT " << s.time << endl;
    cout << "EXEC_TIME " << alg.getExecutionTime(TIME_COEFF) << endl;
    cout << "SOL_TIME " << alg.getBestSolutionTime(TIME_COEFF) << endl;
    cout << "SEED " << Rng::getSeed() << endl;

    if (argc < 3) return 0;

    // output to file
    string outFile = string(argv[2]);
    string dir = outFile.substr(0, outFile.find_last_of('/'));
    if (dir != outFile) filesystem::create_directories(dir);  // make sure the path exists

    ofstream fout(outFile, ios::out);
    fout << "EXEC_TIME " << alg.getExecutionTime(TIME_COEFF) << endl;
    fout << "SOL_TIME " << alg.getBestSolutionTime(TIME_COEFF) << endl;
    fout << "OBJ " << s.time << endl;
    fout << "SEED " << Rng::getSeed() << endl;
    fout << "N_ROUTES " << s.routes.size() << endl;
    fout << "N_CLIENTS";
    for (auto& r : s.routes) fout << " " << (r->size() - 2);
    fout << endl << "ROUTES" << endl;
    for (auto& r : s.routes) {
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
    for (auto x : alg.getSearchProgress()) {
        spout << (unsigned int)(x.first * TIME_COEFF) << "," << x.second << endl;
    }
    spout.close();
    return 0;
}
