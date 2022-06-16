#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-inefficient-string-concatenation"
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <regex>
#include <iomanip>
#include <limits>

using namespace std;

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

void convertToCsv() {
    string beta, instance, te, ti, opt, bestObj, bestDev, meanObj, meanDev;

    auto archRef = readOptimalFile("Solomon/0arch.txt");
    for (const string& n : vector<string>{"10", "15", "20"}) {
        ifstream fin("output10/Results/Solomon" + n + ".txt", ios::in);
        ofstream fout("outputcsv/Solomon" + n + ".csv", ios::out);
        fout << fixed << setprecision(2);
        while(fin.good()) {
            fin >> beta >> instance >> te >> ti >> opt >> bestObj >> bestDev >> meanObj >> meanDev;
            if(beta == "Better:") break;

            string archObj = "archObj", archGap = "archGap";
            if(beta != "beta") {
                te = to_string((int) ((stoi(te) / (1976.0 / 1201.0)) / 1000));
                ti = to_string((int) ((stoi(ti) / (1976.0 / 1201.0)) / 1000));
                bestDev = bestDev.substr(0, bestDev.size() - 1);
                meanDev = meanDev.substr(0, meanDev.size() - 1);

                string key = n + "/" + instance + "_" + beta; // NOLINT(performance-inefficient-string-concatenation)
                cout << key << endl;
                unsigned int aObj = archRef[key];
                double aGap = (((double) aObj / stoi(opt)) - 1) * 100;
                archObj = to_string(aObj);

                stringstream stream;
                stream << fixed << setprecision(2) << aGap;
                archGap = stream.str();
            }

            fout << beta << "," << instance << "," << opt << ",,";
            fout << archObj << "," << archGap << ",,";
            fout << bestObj << "," << bestDev << "," << meanObj << "," << meanDev << ",";
            fout << te << "," << ti << endl;
        }
        fout.close();
        fin.close();
    }
}

void convertToCsv2() {
    string beta, instance, te, ti, archObj, bestObj, bestDev, meanObj, meanDev;

    for (const string& n : vector<string>{"50", "100"}) {
        ifstream fin("output10/Results/Solomon" + n + ".txt", ios::in);
        ofstream fout("outputcsv/Solomon" + n + ".csv", ios::out);
        while (fin.good()) {
            fin >> beta >> instance >> te >> ti >> archObj >> bestObj >> bestDev >> meanObj >> meanDev;
            if (beta == "Better:") break;

            if (beta != "beta") {
                te = to_string((int) (stoi(te) / (1976.0 / 1201.0)) / 1000);
                ti = to_string((int) (stoi(ti) / (1976.0 / 1201.0)) / 1000);
                bestDev = bestDev.substr(0, bestDev.size() - 1);
                meanDev = meanDev.substr(0, meanDev.size() - 1);
            }

            fout << beta << "," << instance << ",,";
            fout << archObj << ",,,";
            fout << bestObj << "," << bestDev << "," << meanObj << "," << meanDev << ",";
            fout << te << "," << ti << endl;
        }
        fout.close();
        fin.close();
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "performance-inefficient-string-concatenation"
void convertToCsvTSPLIB() {
    vector<string> names(
            {"eil51", "berlin52", "st70", "eil76", "pr76", "rat99", "kroA100", "kroB100", "kroC100", "kroD100",
             "kroE100", "rd100", "eil101", "lin105", "pr107", "pr124", "bier127", "ch130", "pr136", "pr144", "ch150",
             "kroA150", "kroB150", "pr152", "u159", "rat195", "d198", "kroA200", "kroB200", "ts225", "tsp225", "pr226",
             "gil262", "pr264", "a280", "pr299", "lin318", "rd400", "fl417", "pr439", "pcb442", "d493"});
    vector<string> betas({"0.5", "1", "1.5", "2", "2.5", "3"});

    string aux;
    unsigned int obj;

    map<string, unsigned int> archObjs = readOptimalFile("TSPLIB/0ptimal.txt");

    for(auto &beta: betas) {
        ofstream fout("outputcsv/TSPLIB_" + beta + ".csv", ios::out);
        fout << fixed << setprecision(2);
        for(auto &name: names) {
            string instance = name + "_" + beta;
            unsigned int bestObj = numeric_limits<unsigned int>::max();
            unsigned int sumObj = 0, sumTI = 0, sumTE = 0;
            for(unsigned int i = 1; i <= 10; i++) {
                string path = "output10/Results_" + beta + "/TSPLIB/" + instance + "_" + to_string(i) + ".txt";
                ifstream fin(path, ios::in);

                fin >> aux >> obj;
                sumTE += obj;

                fin >> aux >> obj;
                sumTI += obj;

                fin >> aux >> obj;
                sumObj += obj;
                bestObj = min(bestObj, obj);

                fin.close();
            }

            double meanObj = sumObj / 10.0;
            unsigned int meanTE = (unsigned int) ((sumTE / 10.0) / (1976.0 / 1201.0)) / 1000;
            unsigned int meanTI = (unsigned int) ((sumTI / 10.0) / (1976.0 / 1201.0)) / 1000;

            double gapBest = (((double) bestObj / archObjs[instance]) - 1) * 100;
            double gapMean = (((double) meanObj / archObjs[instance]) - 1) * 100;

            fout << name << ",," << archObjs[instance] << ",,";
            fout << bestObj << "," << gapBest << ",,";
            fout << meanObj << "," << gapMean << ",";
            fout << meanTE << "," << meanTI << endl;
        }
        fout.close();
    }
}
#pragma clang diagnostic pop

void convertToCsvATSPLIB() {
    string beta, instance, te, ti, archObj, bestObj, bestDev, meanObj, meanDev;

    struct Data {
        string beta, instance, te, ti, archObj, bestObj, bestDev, meanObj, meanDev;
    };

    vector<string> betas({"0.5", "1", "1.5", "2", "2.5", "3"});
    map<string, vector<Data> > datas;
    for(const auto &b: betas)
        datas[b] = vector<Data>();

    ifstream fin("output10/Results/aTSPLIB.txt", ios::in);
    ofstream fout("outputcsv/aTSPLIB.csv", ios::out);
    while(fin.good()) {
        fin >> beta >> instance >> te >> ti >> archObj >> bestObj >> bestDev >> meanObj >> meanDev;
        if(beta == "Better:") break;

        if(beta != "beta") {
            te = to_string((int) (stoi(te) / (1976.0 / 1201.0)) / 1000);
            ti = to_string((int) (stoi(ti) / (1976.0 / 1201.0)) / 1000);
            bestDev = bestDev.substr(0, bestDev.size() - 1);
            meanDev = meanDev.substr(0, meanDev.size() - 1);

            Data data = {beta, instance, te, ti, archObj, bestObj, bestDev, meanObj, meanDev};
            datas[beta].push_back(data);
        } else {
            fout << beta << "," << instance << ",,";
            fout << archObj << ",,,";
            fout << bestObj << "," << bestDev << "," << meanObj << "," << meanDev << ",";
            fout << te << "," << ti << endl;
        }
    }
    fin.close();

    for(auto const &b: betas) {
        for(const auto &d: datas[b]) {
            fout << d.beta << "," << d.instance << ",,";
            fout << d.archObj << ",,,";
            fout << d.bestObj << "," << d.bestDev << "," << d.meanObj << "," << d.meanDev << ",";
            fout << d.te << "," << d.ti << endl;
        }
    }
    fout.close();
}

void tsplibStats() {
    vector<string> names(
            {"eil51", "berlin52", "st70", "eil76", "pr76", "rat99", "kroA100", "kroB100", "kroC100", "kroD100",
             "kroE100", "rd100", "eil101", "lin105", "pr107", "pr124", "bier127", "ch130", "pr136", "pr144", "ch150",
             "kroA150", "kroB150", "pr152", "u159", "rat195", "d198", "kroA200", "kroB200", "ts225", "tsp225", "pr226",
             "gil262", "pr264", "a280", "pr299", "lin318", "rd400", "fl417", "pr439", "pcb442", "d493"});
    vector<string> betas({"0.5", "1", "1.5", "2", "2.5", "3"});

    vector<pair<unsigned int, unsigned int> > intervals({
                                                                {50, 100},
                                                                {101, 150},
                                                                {151, 250},
                                                                {251, 500},

    });

    map<string, unsigned int> archObjs = readOptimalFile("TSPLIB/0ptimal.txt");
    string aux;
    unsigned int obj;

    for(const auto &interval: intervals) {
        unsigned int qnt = 0, qntArchBest = 0, qntMyBest = 0, meanTE = 0, meanTI = 0;
        double sumGaps = 0.0;
        for(const auto &name: names) {
            unsigned int n = stoi(regex_replace(name, regex("[^0-9]*([0-9]+).*"), string("$1")));
            if(n < interval.first || n > interval.second) continue;

            for(const auto &beta: betas) {
                qnt++;

                string instance = name + "_" + beta;
                unsigned int bestObj = numeric_limits<unsigned int>::max();
                unsigned int sumTI = 0, sumTE = 0, meanObj = 0;
                for(unsigned int i = 1; i <= 10; i++) {
                    string path = "output/0Result/Results/TSPLIB/" + instance + "_" + to_string(i) + ".txt";
                    ifstream fin(path, ios::in);
                    fin >> aux >> obj;
                    sumTE += obj;
                    fin >> aux >> obj;
                    sumTI += obj;
                    fin >> aux >> obj;
                    meanObj += obj;
                    bestObj = min(bestObj, obj);
                    fin.close();
                }


                if(archObjs[instance] < bestObj) qntArchBest++;
                else if (bestObj < archObjs[instance]) qntMyBest++;

                meanObj /= 10;
                meanTE += (unsigned int) ((sumTE / 10.0) / (1976.0 / 1201.0)) / 1000;
                meanTI += (unsigned int) ((sumTI / 10.0) / (1976.0 / 1201.0)) / 1000;
                double gapBest = (((double) bestObj / archObjs[instance]) - 1) * 100;
                double gapMean = (((double) meanObj / archObjs[instance]) - 1) * 100;
                sumGaps += gapBest;
            }
        }
        cout << interval.first << "-" << interval.second << endl;
        cout << "qnt: " << qnt << endl;
        cout << "qntArch: " << qntArchBest << "   ";
        cout << "qntMy: " << qntMyBest << endl;
        cout << "TM: " << to_string(meanTI / qnt) << endl;
        cout << "gap: " << to_string(sumGaps / qnt) << endl;
        cout << endl;
    }

}

int main(int argc, char **argv) {
    cout << fixed << setprecision(2);

    convertToCsvATSPLIB();
    return 0;
}
#pragma clang diagnostic pop
