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
#include <sys/stat.h>

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
map<string, string> readValues(stringstream &in) {
    string key, value;
    map<string, string> values;
    while (in.good()) {
        in >> key >> value;
        values[key] = value;
    }
    return values;
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

bool pathExists(const string &s)
{
    struct stat buffer;
    return (stat (s.c_str(), &buffer) == 0);
}

void runInstances(const vector<Instance> &instances, const string &executionId, const string &outputFolder) {
//    unsigned long long timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
//            std::chrono::system_clock::now().time_since_epoch()).count();

    char buffer[512];
    sprintf(buffer, "output/%s/%s.txt", outputFolder.c_str(), executionId.c_str()); // output file
    ofstream fout(buffer, ios::out);

    cout << fixed;
    cout.precision(2);

    cout << "beta   Instance   TE(ms)  TI(ms)    Obj     opt   dev" << endl;
    fout << "beta   Instance   TE(ms)  TI(ms)    Obj     opt   dev" << endl;

    unsigned int better = 0, worse = 0, same = 0;

    for (auto &instance: instances) {
        sprintf(buffer, "./TSPrd %s %s", instance.file.c_str(), outputFolder.c_str());
        stringstream stream(execute(buffer));

        map<string, string> values = readValues(stream);

        if (values.count("ERROR") > 0) {
            string error = "Error: " + values["ERROR"];
            sprintf(buffer, "%3s  %10s   %s", instance.beta.c_str(), instance.name.c_str(), error.c_str());
            cout << buffer << endl;
            fout << buffer << endl;
            continue;
        }

        unsigned int result = stoi(values["RESULT"]);
        unsigned int executionTime = stoi(values["EXEC_TIME"]);
        unsigned int bestSolutionTime = stoi(values["SOL_TIME"]);

        if (result < instance.optimal) better++;
        else if (result > instance.optimal) worse++;
        else same++;

        double deviation = (((double) result / instance.optimal) - 1) * 100;

        sprintf(buffer, "%3s  %10s  %6d  %6d  %6d  %6d  % 2.2f%%", instance.beta.c_str(), instance.name.c_str(),
                executionTime, bestSolutionTime, result, instance.optimal, deviation);

        cout << buffer << endl;
        fout << buffer << endl;
        fout.flush();
    }
    cout << "Better: " << better << "  |  Worse: " << worse << "  |  Same: " << same << endl;
    fout << "Better: " << better << "  |  Worse: " << worse << "  |  Same: " << same << endl;

    fout.close();
}

void runSolomonInstances(const string &outputFolder) {
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
        runInstances(instances, "Solomon" + to_string(n), outputFolder);
    }
}

void runTSPLIBInstances(const string &outputFolder) {
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

    runInstances(instances, "TSPLIB", outputFolder);
}

void runATSPLIBInstances(const string &outputFolder) {
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

    runInstances(instances, "aTSPLIB", outputFolder);
}

int main() {
    string outputFolder = "2021.05.05_15.40";

    if(pathExists("output/" + outputFolder)) {
        throw invalid_argument("output dir already exists!");
    } else {
        system(("mkdir -p output/" + outputFolder).c_str());
    }

    runSolomonInstances(outputFolder);
    runTSPLIBInstances(outputFolder);
    runATSPLIBInstances(outputFolder);
    return 0;
}
