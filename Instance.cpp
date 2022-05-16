#include "Instance.h"
#include <fstream>
#include <iostream>
#include <cmath>

// read the stream until 's' appear
void readUntil(ifstream &in, const string &s) {
    string x;
    while (x != s) {
        in >> x;
    }
}

void floydWarshall(vector<vector<unsigned int> > &W) {
    // apply floyd warshall algorithm to ensure triangular inequality
    for (unsigned int k = 0; k < W.size(); k++) {
        for (unsigned int i = 0; i < W.size(); i++) {
            for (unsigned int j = 0; j < W.size(); j++) {
                W[i][j] = min(W[i][j], W[i][k] + W[k][j]);
            }
        }
    }
}

Instance::Instance(const string &instance) : V(0), W(0), RD(0), biggerRD(0), symmetric(false) {

    ifstream in(("instances/" + instance + ".dat").c_str(), ios::in);
    if (!in) {
        cout << "ERROR failed_open_file" << endl;
        exit(1);
    }

    string instanceSet = instance.substr(0, instance.find('/'));
    if (instanceSet == "aTSPLIB") {
        readDistanceMatrixInstance(in);
    } else if (instanceSet == "TSPLIB" || instanceSet == "Solomon" || instanceSet == "testSet") {
        readCoordinatesListInstance(in);
    } else {
        cout << "ERROR unknown_instance_set" << endl;
        exit(1);
    }

    in.close();
}

void Instance::readDistanceMatrixInstance(ifstream &in) {
    readUntil(in, "DIMENSION:");
    in >> V;
    W.resize(V, vector<unsigned int>(V));
    RD.resize(V);

    readUntil(in, "EDGE_WEIGHT_SECTION");
    for (unsigned int i = 0; i < V; i++) {
        for (unsigned int j = 0; j < V; j++) {
            in >> W[i][j];
        }
        W[i][i] = 0;
    }

    biggerRD = 0;
    readUntil(in, "RELEASE_DATES");
    for (unsigned int i = 0; i < V; i++) {
        in >> RD[i];
        if (RD[i] > biggerRD)
            biggerRD = RD[i];
    }

    // verify matrix symmetry
    symmetric = true;
    for (unsigned int i = 0; i < V && symmetric; i++) {
        for (unsigned int j = i + 1; j < V && symmetric; j++) {
            symmetric = W[i][j] == W[j][i];
        }
    }

    floydWarshall(W);
}

void Instance::readCoordinatesListInstance(ifstream &in) {
    symmetric = true;

    readUntil(in, "<DIMENSION>");
    in >> V;

    W.resize(V, vector<unsigned int>(V));
    RD.resize(V);

    readUntil(in, "</VERTICES>");

    vector<double> X(V);
    vector<double> Y(V);
    double aux;

    biggerRD = 0;
    for (unsigned int i = 0; i < V; i++) {
        in >> X[i];
        in >> Y[i];
        in >> aux;
        in >> aux;
        in >> aux;
        in >> aux; // not important data
        in >> RD[i];
        if (RD[i] > biggerRD)
            biggerRD = RD[i];
    }

    // calculate rounded euclidian distances between each pair of vertex
    for (unsigned int i = 0; i < V; i++) {
        W[i][i] = 0;
        for (unsigned j = i + 1; j < V; j++) {
            double a = X[i] - X[j];
            double b = Y[i] - Y[j];

            double distance = sqrt(a * a + b * b);

            W[i][j] = floor(distance + 0.5);
            W[j][i] = W[i][j];
        }
    }

    floydWarshall(W);
}

unsigned int Instance::nVertex() const {
    return V;
}

unsigned int Instance::nClients() const {
    return V - 1;
}

unsigned int Instance::releaseDateOf(unsigned int c) const {
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

