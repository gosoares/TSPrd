#include <iostream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <map>

using namespace std;

#include <array>

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

vector<pair<string, int> > optimals = {
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

int main() {
    cout << fixed;
    cout.precision(2);

    char buffer[512];

    ofstream outf("Output/relatorio.txt", ios::out);

    cout << "  Instance    Obj  t(s)  opt  desv" << endl;
    outf << "  Instance    Obj  t(s)  opt  desv" << endl;

    for (auto &x: optimals) {
        sprintf(buffer, "./TSPrd %s", x.first.c_str());
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

        double desv = (((double) result / x.second) - 1) * 100;
        sprintf(buffer, "%10s  %5d  %2d  %5d  %2.2f%%", x.first.c_str(), result, time / 1000, x.second, desv);

        cout << buffer << endl;
        outf << buffer << endl;
        outf.flush();
    }

    outf.close();
    return 0;

}
