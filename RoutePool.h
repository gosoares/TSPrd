#ifndef TSPRD_ROUTEPOOL_H
#define TSPRD_ROUTEPOOL_H

#include <vector>
#include <unordered_set>
#include "Solution.h"

using namespace std;

struct RouteData {
    vector<unsigned int> route;
    unsigned int releaseTime;
    unsigned int duration;
    unsigned int solTime;

    bool operator==(const RouteData &other) const {

        if(route.size()!= other.route.size() ||
            releaseTime != other.releaseTime || duration != other.duration)
            return false;

        for (unsigned int r = 0; r < route.size(); r++) {
            if (route.at(r) != other.route.at(r))
                return false; // se algum elemento não é igual, então as rotas são diferentes
        }

        return true; // se chegou aqui, as rotas são iguais
    }

    struct Hasher {
    public:
        size_t operator()(const RouteData* r) const {
            size_t h1 = std::hash<unsigned int>{}(r->duration);
            size_t h2 = std::hash<unsigned int>{}(r->route.size());
            size_t h3 = std::hash<unsigned int>{}(r->releaseTime);
            size_t h4 = std::hash<unsigned int>{}(r->route[1]);
            size_t h5 = std::hash<unsigned int>{}(r->route[r->route.size() / 2]);
            size_t h6 = std::hash<unsigned int>{}(r->route[r->route.size() - 2]);
            return (((((h1 ^ (h2 << 1)) ^ (h3 << 2)) ^ (h4 << 3)) ^ (h5 << 4)) ^ (h6 << 5));
        }
    };

    struct Comparator {
    public:
        bool operator()(const RouteData* r1, const RouteData* r2) const {
            return *r1 == *r2;
        }
    };
};

class RoutePool {
public:
    explicit RoutePool(unsigned int maxRoutes);

    void addRoute(RouteData *);
    void addRoutesFrom(const Solution &solution);
    vector<RouteData *> getRoutes();

    static void printRoute(RouteData *route);
    void printPool();


    void test() {
        auto r1 = new RouteData();
        r1->route = vector<unsigned int>({1, 2, 3});
        r1->releaseTime = 10;
        r1->duration = 10;
        r1->solTime = 10;

        auto r2 = new RouteData();
        r2->route = vector<unsigned int>({1, 2, 3, 4});
        r2->releaseTime = 10;
        r2->duration = 10;
        r2->solTime = 5;

        auto r3 = new RouteData();
        r3->route = vector<unsigned int>({1, 2, 3});
        r3->releaseTime = 10;
        r3->duration = 10;
        r3->solTime = 5;

        auto r4 = new RouteData();
        r4->route = vector<unsigned int>({1, 2, 3, 4});
        r4->releaseTime = 10;
        r4->duration = 10;
        r4->solTime = 15;

        auto r5 = new RouteData();
        r5->route = vector<unsigned int>({1, 2, 3, 4, 5});
        r5->releaseTime = 10;
        r5->duration = 10;
        r5->solTime = 3;

        addRoute(r1);
        printPool();
        getchar();
        addRoute(r2);
        printPool();
        getchar();
        addRoute(r3);
        printPool();
        getchar();

        addRoute(r4);
        printPool();
        getchar();

        addRoute(r5);
        printPool();
        getchar();

        auto r = getRoutes();
    }

private:
    unsigned int maxRoutes;
    unordered_set<RouteData *, RouteData::Hasher, RouteData::Comparator> routes;
};

#endif //TSPRD_ROUTEPOOL_H
