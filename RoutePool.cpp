#include "RoutePool.h"
#include <algorithm> 
#include <iostream>

using namespace std;    

RoutePool::RoutePool(unsigned int maxRoutes): maxRoutes(maxRoutes) {
}

void RoutePool::addRoute(RouteData* routeData) {

    pair<unordered_set<RouteData *>::iterator, bool> pointer;
    pointer = routes.insert(routeData);

    if(pointer.second) { // o elemento foi inserido no set
        // o conjunto aumentou de tamanho, é verificado se ultrapassou o tamanho máximo
//        if(routes.size() > maxRoutes) {
//            routes.erase(prev(routes.end())); // FIXME complexidade linear??
//        }
    } else {
        // existe uma rota identica no set, verificamremos o solTime
        auto existing = *(pointer.first);

        if(routeData->solTime < existing->solTime) {
            // se o solTime do atual for menor do que o do existente
            // remove o existente do set, e insere o atual
            // equivale a atualizar o solTime do elemento atual
            routes.erase(existing);
            auto a = routes.insert(routeData);

            delete existing;
        } else {
            delete routeData;
        }

    }
}

void RoutePool::addRoutesFrom(const Solution &solution) {
    for(unsigned int i = 0; i < solution.routes.size(); i++) {
        auto routeData = new RouteData();
        routeData->route = vector<unsigned int>(*(solution.routes[i]));
        routeData->releaseTime = solution.routeRD[i];
        routeData->duration = solution.routeTime[i];
        routeData->solTime = solution.time;

        addRoute(routeData);
    }
}

vector<RouteData *> RoutePool::getRoutes(){
    vector<RouteData *> r;
    r.reserve(routes.size());

    copy(routes.begin(), routes.end(), back_inserter(r));

    return r;
}

void RoutePool::printPool(){
    for (auto r : this->routes) {
        printRoute(r);
    }
    cout << endl << endl;
}

void RoutePool::printRoute(RouteData *route){
    cout << "Solution time: " << route->solTime << endl;
    cout << "Duration: " << route->duration << endl;
    cout << "Release date: " << route->releaseTime << endl;
    cout << "Route: ";
    for(unsigned int i : route->route){
        cout << i << " ";
    }
    cout << endl;
}
