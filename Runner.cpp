#include <iostream>
#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <chrono>
#include <string>
#include <array>
#include <map>

using namespace std;

struct Instance {
    string name; // instance name
    string file; // file name with dir
    string beta; // beta used to generate instance
    unsigned int optimal; // optimal value of solution
};

string execute(const char *cmd) {
    array<char, 128> buffer{};
    string result;
    shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

void runInstances(const vector<Instance> &instances, string executionId = "") {
    if (executionId.empty()) {
        unsigned long long timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        executionId = to_string(timeStamp);
    }
    char buffer[512];

    sprintf(buffer, "output/log_%s.txt", executionId.c_str()); // output file
    ofstream outf(buffer, ios::out);

    cout << "beta  Instance    Obj  t(ms)  opt  dev" << endl;
    outf << "beta  Instance    Obj  t(ms)  opt  dev" << endl;

    for (auto &instance: instances) {
        sprintf(buffer, "./TSPrd %s", instance.file.c_str());
        stringstream stream(execute(buffer));
        string s;

        int time, result;

        while (s != "RESULT") {
            stream >> s;
        }
        stream >> result;

        while (s != "TIME") {
            stream >> s;
        }
        stream >> time;

        double deviation = (((double) result / instance.optimal) - 1) * 100;

        sprintf(buffer, "%3s  %10s  %5d  %2d  %5d  %2.2f%%", instance.beta.c_str(), instance.name.c_str(), result, time, instance.optimal,
                deviation);

        cout << buffer << endl;
        outf << buffer << endl;
        outf.flush();
    }

    outf.close();
}

void runATSPLIBInstances() {
    vector<pair<string, unsigned int> > optimals = {
            {"ftv33_0.5",   1845},
            {"ftv33_1",     2225},
            {"ftv33_1.5",   2560},
            {"ftv33_2",     3094},
            {"ftv33_2.5",   3608},
            {"ftv33_3",     4083},
            {"ft53_0.5",    11660},
            {"ft53_1",      13975},
            {"ft53_1.5",    16081},
            {"ft53_2",      18768},
            {"ft53_2.5",    20875},
            {"ft53_3",      23774},
            {"ftv70_0.5",   3396},
            {"ftv70_1",     3963},
            {"ftv70_1.5",   4824},
            {"ftv70_2",     5468},
            {"ftv70_2.5",   6334},
            {"ftv70_3",     6929},
            {"kro124p_0.5", 50925},
            {"kro124p_1",   62964},
            {"kro124p_1.5", 74083},
            {"kro124p_2",   88904},
            {"kro124p_2.5", 101961},
            {"kro124p_3",   117612},
            {"rbg403_0.5",  4875},
            {"rbg403_1",    5579},
            {"rbg403_1.5",  6215},
            {"rbg403_2",    7477},
            {"rbg403_2.5",  8623},
            {"rbg403_3",    9788}
    };

    vector<Instance> instances;
    instances.reserve(optimals.size());
    for (auto &x: optimals) {
        string file = "aTSPLIB/" + x.first;
        string beta = file.substr(file.find('_') + 1);
        Instance instance = {x.first, file, beta, x.second};
        instances.push_back(instance);
    }

    runInstances(instances);
}

void runSolomonInstances() {
    map<string, unsigned int> optimal = {
            {"10/C101_0.5", 79},
            {"10/C201_0.5", 186},
            {"10/R101_0.5", 217},
            {"10/RC101_0.5", 195},
            {"10/C101_1", 101},
            {"10/C201_1", 221},
            {"10/R101_1", 261},
            {"10/RC101_1", 241},
            {"10/C101_1.5", 113},
            {"10/C201_1.5", 250},
            {"10/R101_1.5", 323},
            {"10/RC101_1.5", 289},
            {"10/C101_2", 134},
            {"10/C201_2", 297},
            {"10/R101_2", 373},
            {"10/RC101_2", 347},
            {"10/C101_2.5", 157},
            {"10/C201_2.5", 338},
            {"10/R101_2.5", 432},
            {"10/RC101_2.5", 395},
            {"10/C101_3", 180},
            {"10/C201_3", 386},
            {"10/R101_3", 495},
            {"10/RC101_3", 444},
            {"15/C101_0.5", 145},
            {"15/C201_0.5", 228},
            {"15/R101_0.5", 288},
            {"15/RC101_0.5", 220},
            {"15/C101_1", 176},
            {"15/C201_1", 271},
            {"15/R101_1", 361},
            {"15/RC101_1", 266},
            {"15/C101_1.5", 217},
            {"15/C201_1.5", 327},
            {"15/R101_1.5", 427},
            {"15/RC101_1.5", 330},
            {"15/C101_2", 261},
            {"15/C201_2", 375},
            {"15/R101_2", 509},
            {"15/RC101_2", 387},
            {"15/C101_2.5", 304},
            {"15/C201_2.5", 446},
            {"15/R101_2.5", 572},
            {"15/RC101_2.5", 439},
            {"15/C101_3", 347},
            {"15/C201_3", 517},
            {"15/R101_3", 663},
            {"15/RC101_3", 494},
            {"20/C101_0.5", 177},
            {"20/C201_0.5", 247},
            {"20/R101_0.5", 326},
            {"20/RC101_0.5", 305},
            {"20/C101_1", 212},
            {"20/C201_1", 285},
            {"20/R101_1", 409},
            {"20/RC101_1", 374},
            {"20/C101_1.5", 260},
            {"20/C201_1.5", 349},
            {"20/R101_1.5", 475},
            {"20/RC101_1.5", 422},
            {"20/C101_2", 306},
            {"20/C201_2", 402},
            {"20/R101_2", 568},
            {"20/RC101_2", 508},
            {"20/C101_2.5", 356},
            {"20/C201_2.5", 478},
            {"20/R101_2.5", 648},
            {"20/RC101_2.5", 586},
            {"20/C101_3", 406},
            {"20/C201_3", 551},
            {"20/R101_3", 750},
            {"20/RC101_3", 654},
            {"50/C101_0.5", 333},
            {"50/C201_0.5", 415},
            {"50/R101_0.5", 588},
            {"50/RC101_0.5", 530},
            {"50/C101_1", 422},
            {"50/C201_1", 528},
            {"50/R101_1", 728},
            {"50/RC101_1", 661},
            {"50/C101_1.5", 520},
            {"50/C201_1.5", 660},
            {"50/R101_1.5", 880},
            {"50/RC101_1.5", 819},
            {"50/C101_2", 616},
            {"50/C201_2", 780},
            {"50/R101_2", 1056},
            {"50/RC101_2", 967},
            {"50/C101_2.5", 715},
            {"50/C201_2.5", 919},
            {"50/R101_2.5", 1257},
            {"50/RC101_2.5", 1117},
            {"50/C101_3", 814},
            {"50/C201_3", 1070},
            {"50/R101_3", 1453},
            {"50/RC101_3", 1281},
            {"100/C101_0.5", 677},
            {"100/C201_0.5", 735},
            {"100/R101_0.5", 823},
            {"100/RC101_0.5", 839},
            {"100/C101_1", 880},
            {"100/C201_1", 945},
            {"100/R101_1", 1004},
            {"100/RC101_1", 1035},
            {"100/C101_1.5", 1076},
            {"100/C201_1.5", 1143},
            {"100/R101_1.5", 1229},
            {"100/RC101_1.5", 1261},
            {"100/C101_2", 1287},
            {"100/C201_2", 1365},
            {"100/R101_2", 1478},
            {"100/RC101_2", 1507},
            {"100/C101_2.5", 1508},
            {"100/C201_2.5", 1574},
            {"100/R101_2.5", 1721},
            {"100/RC101_2.5", 1778},
            {"100/C101_3", 1700},
            {"100/C201_3", 1808},
            {"100/R101_3", 1991},
            {"100/RC101_3", 2059}
    };

    vector<unsigned int> ns({10, 15, 20, 50, 100});
    vector<string> names({"C101", "C201", "R101", "RC101"});
    vector<string> betas({"0.5", "1", "1.5", "2", "2.5", "3"});

    vector<Instance> instances;
    instances.reserve(betas.size() * names.size());
    for (auto &n: ns) {
        instances.clear();
        for (auto &beta: betas) {
            for (auto &name: names) {
                string file = to_string(n) + "/" + name + "_" + beta; // NOLINT(performance-inefficient-string-concatenation)
                Instance instance = {name, "Solomon/" + file, beta, optimal[file]};
                instances.push_back(instance);
            }
        }

        cout << "for n = " << n << endl;
        runInstances(instances);
    }
}

int main() {
    cout << fixed;
    cout.precision(2);

    runSolomonInstances();

    return 0;
}
