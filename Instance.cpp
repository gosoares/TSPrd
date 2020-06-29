#include "Instance.h"
#include <fstream>
#include <iostream>

void readUntil(ifstream& in, const string& s) {
    string x;
    while(x != s) {
        in >> x;
    }
}

Instance::Instance(const string& instance): N(0), W(0), RD(0) {

    ifstream in(("../instances/" + instance).c_str(), ios::in);
    if(!in){
        std::cout << "Falha ao abrir o arquivo!" << endl;
        exit(1);
    }

    readUntil(in, "DIMENSION:");
    in >> N;
    W.resize(N, vector<unsigned int>(N));
    RD.resize(N);

    readUntil(in, "EDGE_WEIGHT_SECTION");
    for(int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            in >> W[i][j];
        }
    }

    readUntil(in, "RELEASE_DATES");
    for (int i = 0; i < N; i++) {
        in >> RD[i];
    }

    in.close();

}

unsigned int Instance::nClients() const {
    return N;
}

unsigned int Instance::releaseTimeOf(unsigned int c) const {
    return RD[c];
}

unsigned int Instance::time(unsigned int i, unsigned int j) const {
    return W[i][j];
}
