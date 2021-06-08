#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

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
    ifstream fin("output/0Result/Results/Solomon10.txt", ios::in);
    ofstream fout("output/0Result/Solomon10.csv", ios::out);
    while(fin.good()) {
        fin >> beta >> instance >> te >> ti >> opt >> bestObj >> bestDev >> meanObj >> meanDev;
        if(beta == "Better:") break;

        string archObj = "archObj", archGap = "archGap";
        if(beta != "beta") {
            te = to_string(stoi(te) / 1000);
            ti = to_string(stoi(ti) / 1000);
            bestDev = bestDev.substr(0, bestDev.size() - 1);
            meanDev = meanDev.substr(0, meanDev.size() - 1);

            string key = "10/" + instance + "_" + beta; // NOLINT(performance-inefficient-string-concatenation)
            cout << key << endl;
            unsigned int aObj = archRef[key];
            double aGap = (((double) aObj / stoi(opt)) - 1) * 100;
            archObj = to_string(aObj);

            stringstream stream;
            stream << fixed << setprecision(2) << aGap;
            archGap = stream.str();
        }

        fout << beta << ";" << instance << ";" << opt << ";;";
        fout << archObj << ";" << archGap << ";;";
        fout << bestObj << ";" << bestDev << ";" << meanObj << ";" << meanDev << ";";
        fout << te << ";" << ti << endl;
    }
    fout.close();
    fin.close();
}

int main(int argc, char **argv) {
    cout << fixed << setprecision(2);

    convertToCsv();
    return 0;
}