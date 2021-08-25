#ifndef TSPRD_ROUTEPOOL_H
#define TSPRD_ROUTEPOOL_H

#include <vector>
#include "Solution.h"

using namespace std;

struct hashCode
{
    int duration;
    int size;
    int solTime;
    int releaseTime;
    int first;
    int middle;
    int last;
};

// custom specialization of std::hash for my Route
namespace std
{
    template <>
    struct hash<hashCode>
    {
        std::size_t operator()(hashCode const &s) const noexcept
        {
            size_t h1 = hash<int>{}(s.duration);
            size_t h2 = hash<int>{}(s.size);
            size_t h3 = hash<int>{}(s.solTime);
            size_t h4 = hash<int>{}(s.releaseTime);
            size_t h5 = hash<int>{}(s.first);
            size_t h6 = hash<int>{}(s.middle);
            size_t h7 = hash<int>{}(s.last);
            return (((((h1 ^ (h2 << 1)) ^ (h3 << 2)) ^ (h4 << 3)) ^ (h5 << 4)) ^ (h6 << 5)) ^ (h7 << 6);
        }
    };
}

struct RouteData {
    vector<unsigned int> route;
    unsigned int releaseTime;
    unsigned int duration;
    unsigned int solTime;
    unsigned long long int hash;
};



class RoutePool {
public:
    struct RoutePtrComp
    {
        bool operator()(RouteData *lhs, RouteData *rhs)
        {
            //return lhs->hash < rhs->hash;

            for (unsigned int r = 0; r < rhs->route.size(); r++) {
                if (lhs->route.at(r) != rhs->route.at(r))
                    return true;
            }

            if (lhs->solTime != rhs->solTime || lhs->route.size() != rhs->route.size() || lhs->duration != rhs->duration || lhs->releaseTime != rhs->releaseTime)
                return true;

            
            
            return false;
        }
    };

    explicit RoutePool(unsigned int maxRoutes, int nClients);
    unsigned int maxRoutes;
    vector<RouteData *> routes;
    set<RouteData *, RoutePtrComp> routesSet;
    int nClients;


    void addRoutesFrom(const Solution &solution);
    unsigned long long int getHash(RouteData *route);
    void printRoute(RouteData *route);
    void printPool();
    void setToVector();

};

#endif //TSPRD_ROUTEPOOL_H
