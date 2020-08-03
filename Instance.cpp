#include "Instance.h"
#include <fstream>
#include <iostream>

void readUntil(ifstream& in, const string& s) {
    string x;
    while(x != s) {
        in >> x;
    }
}

Instance::Instance(const string& instance): V(0), W(0), RD(0) {

    ifstream in(("instances/" + instance + ".dat").c_str(), ios::in);
    if(!in){
        std::cout << "Falha ao abrir o arquivo!" << endl;
        exit(1);
    }

    readUntil(in, "DIMENSION:");
    in >> V;
    W.resize(V, vector<unsigned int>(V));
    RD.resize(V);

    readUntil(in, "EDGE_WEIGHT_SECTION");
    for(int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            in >> W[i][j];
        }
    }

    biggerRD = 0;
    readUntil(in, "RELEASE_DATES");
    for (int i = 0; i < V; i++) {
        in >> RD[i];
        if(RD[i] > biggerRD)
            biggerRD = RD[i];
    }

    in.close();
}

unsigned int Instance::nVertex() const {
    return V;
}

unsigned int Instance::nClients() const {
    return V - 1;
}

unsigned int Instance::releaseTimeOf(unsigned int c) const {
    return RD[c];
}

unsigned int Instance::time(unsigned int i, unsigned int j) const {
    return W[i][j];
}

const vector<vector<unsigned int> > &Instance::getW() const {
    return W;
}

const vector<unsigned int> &Instance::getRD() const {
    return RD;
}

