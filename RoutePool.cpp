#include "RoutePool.h"
#include <algorithm> 
#include <iostream>

using namespace std;    

RoutePool::RoutePool(unsigned int maxRoutes): maxRoutes(maxRoutes) {}

void RoutePool::addRoutesFrom(const Solution &solution) {
    for(unsigned int i = 0; i < solution.routes.size(); i++) {
        auto routeData = new RouteData();
        routeData->route = vector<unsigned int>(*(solution.routes[i]));
        routeData->releaseTime = solution.routeRD[i];
        routeData->duration = solution.routeTime[i];
        routeData->solTime = solution.time;

        routeData->hash = getHash(routeData);
        //printRoute(routeData);

        //getchar();
        pair<set<RouteData *>::iterator, bool> pointer;

        pointer = this->routesSet.insert(routeData);
        if (!pointer.second)
            delete routeData;
    }
}

unsigned long long int RoutePool::getHash(RouteData *route)
{
    hashCode hClient = {
        (int)route->duration,
        (int)route->route.size(),
        (int)route->solTime,
        (int)route->releaseTime,
        (int)route->route[1],
        (int)route->route[route->route.size() / 2],
        (int)route->route[route->route.size() - 2]};
    hash<hashCode> hNumber;
    return hNumber(hClient);
}

void RoutePool::setToVector(){
    for (auto r : this->routesSet) {
        this->routes.push_back(new RouteData(*r));
        if(this->routes.size() == this->maxRoutes){
            break;
        }
    }
}

void RoutePool::printRoute(RouteData *route){
    cout << "Solution time: " << route->solTime << endl;
    cout << "Duration: " << route->duration << endl;
    cout << "Release date: " << route->releaseTime << endl;
    cout << "Hash: " << route->hash << endl;
    cout << "Route: ";
    for(int i = 0; i < route->route.size(); i++){
        cout << route->route[i] << " ";
    }
    cout << endl;
}