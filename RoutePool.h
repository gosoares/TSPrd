#ifndef TSPRD_ROUTEPOOL_H
#define TSPRD_ROUTEPOOL_H

#include <vector>
#include "Solution.h"

using namespace std;

struct RouteData {
    vector<unsigned int> route;
    unsigned int releaseTime;
    unsigned int duration;
    unsigned int solTime;
};

class RoutePool {
public:
    explicit RoutePool(unsigned int maxRoutes);

    unsigned int maxRoutes;

    vector<RouteData *> routes;

    void addRoutesFrom(const Solution &solution);
};

#endif //TSPRD_ROUTEPOOL_H
