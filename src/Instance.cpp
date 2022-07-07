#include "Instance.h"

// read the stream until 's' appear
void readUntil(std::ifstream& in, const std::string& s) {
    std::string x;
    while (x != s) {
        in >> x;
    }
}

void floydWarshall(std::vector<std::vector<int> >& W) {
    // apply floyd warshall algorithm to ensure triangular inequality
    for (int k = 0; k < W.size(); k++) {
        for (int i = 0; i < W.size(); i++) {
            for (int j = 0; j < W.size(); j++) {
                W[i][j] = std::min<int>(W[i][j], W[i][k] + W[k][j]);
            }
        }
    }
}

Instance::Instance(const std::string& instanceFile) : V(0), timesMatrix(0), releaseDates(0) {
    std::ifstream fin(instanceFile, std::ios::in);
    auto firstChar = fin.peek();

    if (firstChar == '<') {  // instance from TSPLIB or Solomon, given by the distance matrix
        readCoordinatesListInstance(fin);
    } else if (firstChar == 'N') {  // instance from aTSPLIB, given by a list of euclidian coordinates
        readDistanceMatrixInstance(fin);
    } else {
        std::cout << "Unknown instance file format." << std::endl;
        exit(1);
    }

    fin.close();
}

void Instance::readDistanceMatrixInstance(std::ifstream& in) {
    readUntil(in, "DIMENSION:");
    in >> V;
    timesMatrix.resize(V, std::vector<int>(V));
    releaseDates.resize(V);

    readUntil(in, "EDGE_WEIGHT_SECTION");
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            in >> timesMatrix[i][j];
        }
        timesMatrix[i][i] = 0;
    }

    readUntil(in, "RELEASE_DATES");
    for (int i = 0; i < V; i++) {
        in >> releaseDates[i];
    }

    floydWarshall(timesMatrix);

    // verify matrix symmetry
    symmetric = true;
    for (int i = 0; i < V && symmetric; i++) {
        for (int j = i + 1; j < V && symmetric; j++) {
            symmetric = timesMatrix[i][j] == timesMatrix[j][i];
        }
    }
}

void Instance::readCoordinatesListInstance(std::ifstream& in) {
    symmetric = true;

    readUntil(in, "<DIMENSION>");
    in >> V;

    timesMatrix.resize(V, std::vector<int>(V));
    releaseDates.resize(V);

    readUntil(in, "</VERTICES>");

    std::vector<double> X(V);
    std::vector<double> Y(V);
    double aux;

    for (int i = 0; i < V; i++) {
        in >> X[i];
        in >> Y[i];
        in >> aux;
        in >> aux;
        in >> aux;
        in >> aux;  // not important data
        in >> releaseDates[i];
    }

    // calculate rounded euclidian distances between each pair of vertex
    for (int i = 0; i < V; i++) {
        timesMatrix[i][i] = 0;
        for (int j = i + 1; j < V; j++) {
            double a = X[i] - X[j];
            double b = Y[i] - Y[j];

            double distance = std::sqrt(a * a + b * b);

            timesMatrix[i][j] = std::floor(distance + 0.5);
            timesMatrix[j][i] = timesMatrix[i][j];
        }
    }

    floydWarshall(timesMatrix);
}
