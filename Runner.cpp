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

// read the integer after the string s appear
unsigned int readInt(stringstream &in, const string &s) {
    string x;
    do {
        in >> x;
    } while (x != s);
    unsigned int value;
    in >> value;
    return value;
}

map<string, unsigned int> readOptimalFile(const string &location) {
    map<string, unsigned int> optimal;
    ifstream fin("instances/" + location, ios::in);
    string instName;
    unsigned int opt;
    while (fin.good()) {
        fin >> instName >> opt;
        optimal[instName] = opt;
    }
    fin.close();
    return optimal;
}

void runInstances(const vector<Instance> &instances, string executionId = "") {
    if (executionId.empty()) {
        unsigned long long timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
        executionId = to_string(timeStamp);
    }
    char buffer[512];

    sprintf(buffer, "output/log_%s.txt", executionId.c_str()); // output file
    ofstream fout(buffer, ios::out);

    cout << fixed;
    cout.precision(2);

    cout << "beta   Instance   TE(ms)  TI(ms)   Obj    opt   dev" << endl;
    fout << "beta   Instance   TE(ms)  TI(ms)   Obj    opt   dev" << endl;

    unsigned int better = 0, worse = 0, same = 0;

    for (auto &instance: instances) {
        sprintf(buffer, "./TSPrd %s", instance.file.c_str());
        stringstream stream(execute(buffer));
        string s;

        unsigned int result, executionTime, bestSolutionTime;

        result = readInt(stream, "RESULT");
        executionTime = readInt(stream, "EXEC_TIME");
        bestSolutionTime = readInt(stream, "SOL_TIME");

        if (result < instance.optimal) better++;
        else if (result > instance.optimal) worse++;
        else same++;

        double deviation = (((double) result / instance.optimal) - 1) * 100;

        sprintf(buffer, "%3s  %10s  %6d  %6d  %5d  %5d  % 2.2f%%", instance.beta.c_str(), instance.name.c_str(),
                executionTime, bestSolutionTime, result, instance.optimal, deviation);

        cout << buffer << endl;
        fout << buffer << endl;
        fout.flush();
    }
    cout << "Better: " << better << "  |  Worse: " << worse << "  |  Same: " << same << endl;
    fout << "Better: " << better << "  |  Worse: " << worse << "  |  Same: " << same << endl;

    fout.close();
}

void runSolomonInstances() {
    map<string, unsigned int> optimal = readOptimalFile("Solomon/0ptimal.txt");

    vector<unsigned int> ns({10, 15, 20, 50, 100});
    vector<string> names({"C101", "C201", "R101", "RC101"});
    vector<string> betas({"0.5", "1", "1.5", "2", "2.5", "3"});

    vector<Instance> instances;
    instances.reserve(betas.size() * names.size());
    for (auto &n: ns) {
        instances.clear();
        for (auto &beta: betas) {
            for (auto &name: names) {
                string file =
                        to_string(n) + "/" + name + "_" + beta; // NOLINT(performance-inefficient-string-concatenation)
                Instance instance = {name, "Solomon/" + file, beta, optimal[file]};
                instances.push_back(instance);
            }
        }

        cout << "for n = " << n << endl;
        runInstances(instances);
    }
}

void runTSPLIBInstances() {
    map<string, unsigned int> optimal = readOptimalFile("TSPLIB/0ptimal.txt");

    vector<string> names(
            {"eil51", "berlin52", "st70", "eil76", "pr76", "rat99", "kroA100", "kroB100", "kroC100", "kroD100",
             "kroE100", "rd100", "eil101", "lin105", "pr107", "pr124", "bier127", "ch130", "pr136", "pr144", "ch150",
             "kroA150", "kroB150", "pr152", "u159", "rat195", "d198", "kroA200", "kroB200", "ts225", "tsp225", "pr226",
             "gil262", "pr264", "a280", "pr299", "lin318", "rd400", "fl417", "pr439", "pcb442", "d493"});
    vector<string> betas({"0.5", "1", "1.5", "2", "2.5", "3"});

    vector<Instance> instances(names.size() + betas.size());
    instances.resize(0); // resize but keep allocated memory

    for (auto &beta: betas) {
        for (auto &name: names) {
            string file = name + "_" + beta; // NOLINT(performance-inefficient-string-concatenation)
            Instance instance = {name, "TSPLIB/" + file, beta, optimal[file]};
            instances.push_back(instance);
        }
    }

    runInstances(instances);
}

void runATSPLIBInstances() {
    map<string, unsigned int> optimal = readOptimalFile("aTSPLIB/0ptimal.txt");

    vector<string> names({"ftv33", "ft53", "ftv70", "kro124p", "rbg403"});
    vector<string> betas({"0.5", "1", "1.5", "2", "2.5", "3"});

    vector<Instance> instances(names.size() + betas.size());
    instances.resize(0); // resize but keep allocated memory

    for (auto &name: names) {
        for (auto &beta: betas) {
            string file = name + "_" + beta; // NOLINT(performance-inefficient-string-concatenation)
            Instance instance = {name, "aTSPLIB/" + file, beta, optimal[file]};
            instances.push_back(instance);
        }
    }

    runInstances(instances);
}

int main() {
    runSolomonInstances();
//    runTSPLIBInstances();
//    runATSPLIBInstances();
    return 0;
}
