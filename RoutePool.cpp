#include "RoutePool.h"

RoutePool::RoutePool(unsigned int maxRoutes): maxRoutes(maxRoutes) {}

void RoutePool::addRoutesFrom(const Solution &solution) {
    for(unsigned int i = 0; i < solution.routes.size(); i++) {
        auto routeData = new RouteData();
        routeData->route = vector<unsigned int>(*(solution.routes[i]));
        routeData->releaseTime = solution.routeRD[i];
        routeData->duration = solution.routeTime[i];
        routeData->solTime = solution.time;

        this->routes.push_back(routeData);
    }
}
