#ifndef TSPRD_INSTANCE_H
#define TSPRD_INSTANCE_H

#include <math.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Instance {
    void readDistanceMatrixInstance(std::ifstream& in);
    void readCoordinatesListInstance(std::ifstream& in);

   public:
    int V;                                       // number of vertices including depot
    std::vector<std::vector<int> > timesMatrix;  // time between each pair of vertex
    std::vector<int> releaseDates;               // release date of each vertex
    bool symmetric;                              // whether `timesMatrix` is symmetric

    explicit Instance(const std::string& filename);
};

#endif  // TSPRD_INSTANCE_H
