#ifndef TSPRD_PARAMETERTUNING_CPP
#define TSPRD_PARAMETERTUNING_CPP

#include <map>
#include <iostream>
#include "GeneticAlgorithm.h"
#include <experimental/filesystem>
#include <regex>
#include <fstream>

namespace filesystem = experimental::filesystem;

struct Result {
    unsigned int obj;
    unsigned int timeBest;
    unsigned int timeStop;
};

struct Params {
    unsigned int mi;
    unsigned int lambda;
    double el;
    double nc;
    unsigned int itNi;

    unsigned int nClose() const {
        return (unsigned int) (nc * mi);
    }

    unsigned int nbElit() const {
        return (unsigned int) (el * mi);
    }

    unsigned int itDiv() const {
        return (unsigned int) (0.4 * itNi);
    }

    void print() const {
        cout << "mi: " << mi << "  lambda: " << lambda;
        cout << "  el: " << el << "  nc: " << nc << endl;
    }
};

static void copyInstances() {
    vector<string> paths = vector<string>({"TSPLIB", "aTSPLIB"});

    for (auto &p : paths) {
        string path = "instances/" + p;
        for (const auto &f: filesystem::directory_iterator(path)) {
            string filename = f.path().filename();
            if (filename.at(0) == '0') continue;

            unsigned int n = stoi(regex_replace(filename, regex("[^0-9]*([0-9]+).*"), string("$1")));

            if (n >= 150 && n <= 300) {
                filesystem::copy_file(f.path(), "instances/testSet/" + filename);
            }
        }
    }
}

static void selectInstances() {
    vector<string> instances;
    for (const auto &entry : filesystem::directory_iterator("instances/testSet")) {
        instances.push_back(entry.path());
    }

    if (instances.size() != 96)
        exit(1);

    sort(instances.begin(), instances.end());

    mt19937 generator((random_device()) ());
    uniform_int_distribution<int> dist(0, 5);

    for (int i = 0; i < instances.size(); i += 6) {
        int keep1 = dist(generator);
        int keep2;
        do {
            keep2 = dist(generator);
        } while (keep1 == keep2);
        for (int j = 0; j < 6; j++) {
            if (j != keep1 && j != keep2) {
                filesystem::remove(instances[i + j]);
            }
        }
        cout << endl << endl;
    }
}

Result runWith(const Instance &instance, const Params &params) {
    auto ga = GeneticAlgorithm(instance, params.mi, params.lambda, params.nClose(),
                               params.nbElit(), params.itNi, params.itDiv(), 60 * 60);

    return {ga.getSolution().time, ga.getBestSolutionTime(), ga.getExecutionTime()};
}

void saveOptionalValues() {
//    vector<pair<unsigned int, string>> instances;
//    for (const auto &entry : filesystem::directory_iterator("instances/testSet")) {
//        string filename = entry.path().filename().string();
//        if(filename.at(0) != '0') {
//            unsigned int n = stoi(regex_replace(filename, regex("[^0-9]*([0-9]+).*"), string("$1")));
//            instances.emplace_back(n, filename.substr(0, filename.find_last_of('.')));
//        }
//    }
//    sort(instances.begin(), instances.end());
//    for(auto i : instances) {
//        cout << '"' <<  i.second << "\", ";
//    }

    vector<string> instances(
            {"ch150_0.5", "ch150_1", "ch150_1.5", "ch150_2", "ch150_2.5", "ch150_3", "kroA150_0.5", "kroA150_1",
             "kroA150_1.5", "kroA150_2", "kroA150_2.5", "kroA150_3", "kroB150_0.5", "kroB150_1", "kroB150_1.5",
             "kroB150_2", "kroB150_2.5", "kroB150_3", "pr152_0.5", "pr152_1", "pr152_1.5", "pr152_2", "pr152_2.5",
             "pr152_3", "u159_0.5", "u159_1", "u159_1.5", "u159_2", "u159_2.5", "u159_3", "rat195_0.5", "rat195_1",
             "rat195_1.5", "rat195_2", "rat195_2.5", "rat195_3", "d198_0.5", "d198_1", "d198_1.5", "d198_2", "d198_2.5",
             "d198_3", "kroA200_0.5", "kroA200_1", "kroA200_1.5", "kroA200_2", "kroA200_2.5", "kroA200_3",
             "kroB200_0.5", "kroB200_1", "kroB200_1.5", "kroB200_2", "kroB200_2.5", "kroB200_3", "ts225_0.5", "ts225_1",
             "ts225_1.5", "ts225_2", "ts225_2.5", "ts225_3", "tsp225_0.5", "tsp225_1", "tsp225_1.5", "tsp225_2",
             "tsp225_2.5", "tsp225_3", "pr226_0.5", "pr226_1", "pr226_1.5", "pr226_2", "pr226_2.5", "pr226_3",
             "gil262_0.5", "gil262_1", "gil262_1.5", "gil262_2", "gil262_2.5", "gil262_3", "pr264_0.5", "pr264_1",
             "pr264_1.5", "pr264_2", "pr264_2.5", "pr264_3", "a280_0.5", "a280_1", "a280_1.5", "a280_2", "a280_2.5",
             "a280_3", "pr299_0.5", "pr299_1", "pr299_1.5", "pr299_2", "pr299_2.5", "pr299_3"});
    ofstream fout("instances/testSet/0ref.txt", ios::out);

    for (const auto &instanceName: instances) {
        unsigned int bestObj = numeric_limits<unsigned int>::max();
        for (int i = 0; i < 10; i++) {
            Instance instance("testSet/" + instanceName);
            auto result = runWith(instance, {25, 100, 0.4, 0.2, 2000});
            bestObj = min(bestObj, result.obj);
        }
        fout << instanceName << " " << bestObj << endl;
        cout << instanceName << " " << bestObj << endl;
    }

    fout.close();
}

map<string, unsigned int> readOptimalFile() {
    map<string, unsigned int> optimal;
    ifstream fin("instances/testSet/0ref.txt", ios::in);
    string instName;
    unsigned int opt;
    while (fin.good()) {
        fin >> instName >> opt;
        optimal[instName] = opt;
    }
    fin.close();
    return optimal;
}

void testParams(const Params &params, const vector<string> &instances, const map<string, unsigned int> &optimals) {
    params.print();
    const unsigned int NUMBER_EXECUTIONS = 10;
    const unsigned int TOTAL_EXECUTIONS = NUMBER_EXECUTIONS * instances.size();

    double totalGap = 0;
    unsigned totalTimeExec = 0;
    unsigned totalTimeBest = 0;

    int nInstance = 0;
    for (auto const &instanceName : instances) {
        nInstance++;
        Instance instance("testSet/" + instanceName);
        for (unsigned int i = 0; i < NUMBER_EXECUTIONS; i++) {
            cout << "\rRuning: " << nInstance << "/" << instances.size();
            cout << "  " << (i + 1) << "/" << NUMBER_EXECUTIONS << "       ";
            fflush(stdout);
            auto result = runWith(instance, params);
            double gap = (((double) result.obj / optimals.at(instanceName)) - 1) * 100;
            totalGap += gap;
            totalTimeExec += result.timeStop;
            totalTimeBest += result.timeBest;
        }
    }

    double meanGap = totalGap / TOTAL_EXECUTIONS;
    unsigned int meanTimeExec = totalTimeExec / TOTAL_EXECUTIONS;
    unsigned int meanTimeBest = totalTimeBest / TOTAL_EXECUTIONS;

    char buffer[200];
    sprintf(buffer, "(%2d %3d %.2f %.2f) -> %8d %8d % 7.2f%%", params.mi, params.lambda, params.el, params.nc,
            meanTimeExec, meanTimeBest, meanGap);
    cout << buffer << endl;

    ofstream fout("instances/testSet/0results.txt", ios::app);
    fout << buffer << endl;
    fout.close();
}

void run(int which = -1) {
    vector<Params> paramsSet({
                                     {25, 100, 0.4, 0.2, 2000},
                                     {13, 50,  0.4, 0.2, 2000},
                                     {50, 200, 0.4, 0.2, 2000},
                                     {25, 50,  0.4, 0.2, 2000},
                             });
    map<string, unsigned int> optimal = readOptimalFile();
    vector<string> instances;
    instances.reserve(optimal.size());
    for (auto const &x : optimal) instances.push_back(x.first);

    if(which == -1) {
        for (const auto &params: paramsSet) {
            testParams(params, instances, optimal);
        }
    } else {
        testParams(paramsSet[which], instances, optimal);
    }
}

int main(int argc, char **argv) {
    cout << fixed;
    cout.precision(2);
//    copyInstances();
//    saveOptionalValues();

    int which = -1;
    if(argc > 1) {
        which = stoi(argv[1]);
    } else {
        cout << "Runing all params" << endl;
    }

    run(which);
    return 0;
}


// void run() {
//    map<string, double> params;
//    params["mi"] = 25;
//    params["lambda"] = 100;
//    params["el"] = 0.4;
//    params["nc"] = 0.2;
//    params["itNi"] = 500;
//
//    map<string, double> bestDev;
//    for (const auto &k : params) {
//        bestDev[k.first] = numeric_limits<double>::max();
//    }
//
//    map<string, vector<double>> possibleParams;
//    possibleParams["mi"] = {15, 25, 40, 60, 80, 100, 140, 200};
//    possibleParams["lambda"] = {50, 100, 150, 200, 300};
//    possibleParams["el"] = {0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
//    possibleParams["nc"] = {0.05, 0.1, 0.2, 0.3, 0.5};
//
//    for (string which: {"mi", "lambda", "el", "nc"}) {
//        for (double testParam : possibleParams[which]) {
//            double previousParam = params[which];
//            params[which] = testParam;
//
//            double paramDev = 0;
//            for (const auto &inst: instances) {
//                for (int i = 0; i < 1; i++) {
//                    Instance instance(inst.first);
//                    auto r = runWith(instance, params);
//                    double deviation = ((double) r.obj / inst.second) - 1;
//                    paramDev += deviation;
//                }
//            }
//
//            if (bestDev[which] - paramDev > 0.000001) { // if is better than current best
//                bestDev[which] = paramDev;
//            } else {
//                params[which] = previousParam; // restore previous param
//            }
//
//            cout << testParam << " " << paramDev << endl;
//        }
//
//        cout << endl << which << ": " << params[which] << endl;
//    }
//}

#endif //TSPRD_PARAMETERTUNING_CPP
