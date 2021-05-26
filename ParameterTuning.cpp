#ifndef TSPRD_PARAMETERTUNING_CPP
#define TSPRD_PARAMETERTUNING_CPP

#include <map>
#include <iostream>
#include "GeneticAlgorithm.h"
#include <filesystem>
#include <regex>
#include <fstream>

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

    [[nodiscard]] unsigned int nClose() const {
        return (unsigned int) (nc * mi);
    }

    [[nodiscard]] unsigned int nbElit() const {
        return (unsigned int) (el * mi);
    }

    [[nodiscard]] unsigned int itDiv() const {
        return (unsigned int) (0.4 * itNi);
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
//    vector<Params> cenarios = vector<Params>({
//             {25, 100, 0.4, 0.2, 2000},
//             {13, 50, 0.4, 0.2, 2000},
//             {50, 200, 0.4, 0.2, 2000},
//             {25, 50, 0.4, 0.2, 2000},
//     });

    vector<string> instances;
    for (const auto &entry : filesystem::directory_iterator("instances/testSet")) {
        if(entry.path().filename().string().at(0) != '0')
            instances.push_back(entry.path().filename());
    }
    sort(instances.begin(), instances.end());

    ofstream fout("instances/testSet/0ref.txt", ios::out);

    for(const auto &instanceName: instances) {
        unsigned int bestObj = numeric_limits<unsigned int>::max();
        for(int i = 0; i < 10; i++) {
            Instance instance("testSet/" + instanceName.substr(0, instanceName.find_last_of('.')));
            auto result = runWith(instance, {25, 100, 0.4, 0.2, 2000});
            bestObj = min(bestObj, result.obj);
        }
        fout << instanceName << " " << bestObj << endl;
        cout << instanceName << " " << bestObj << endl;
    }

    fout.close();
}

int main() {
    saveOptionalValues();
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
