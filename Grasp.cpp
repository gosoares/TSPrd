#include <numeric>
#include <chrono>
#include <random>
#include "Grasp.h"
#include "NeighborSearch.h"

#define F(R) 1 // index of first client in a route
#define L(R) ((R).size() - 2) // index of last client in a route

Grasp::Grasp(
        const Instance &instance, unsigned int itNi, double alpha, unsigned int timeLimit
) : instance(instance), W(instance.getW()), RD(instance.getRD()), itNi(itNi), alpha(alpha),
    timeLimit(timeLimit) {
    NeighborSearch ns(instance, false);
    bestSolution = Solution::INF();

    beginTime = steady_clock::now();
    const steady_clock::time_point maxTime = beginTime + seconds(this->timeLimit);

    unsigned int iterationsNotImproved = 0;
    while (iterationsNotImproved < this->itNi && steady_clock::now() < maxTime) {
        Solution *newSolution = constructSolution();
        ns.educate(newSolution);

        if (newSolution->time < bestSolution->time) {
            bestSolutionFoundTime = steady_clock::now();
            delete bestSolution;
            bestSolution = newSolution;
            iterationsNotImproved = 0;
        } else {
            delete newSolution;
            iterationsNotImproved++;
        }
    }

    endTime = steady_clock::now();
}

struct Insertion {
    const unsigned int vertex;
    const unsigned int route;
    const unsigned int position;
    const int routeCost;
    const int finalTimeCost;
    const unsigned int newRD; // new release date of 'route' when inserting 'vertex' at 'position'
    const unsigned int newTime; // new time of 'route' when inserting 'vertex' at 'position'
    const unsigned int newRD2; // if inserting a depot (vertex == 0) a new route will be generated
    const unsigned int newTime2;

    Insertion(
            unsigned int vertex, unsigned int route, unsigned int position,
            int routeCost, int finalTimeCost, unsigned int newRD, unsigned int newTime,
            unsigned int newRD2 = 0, unsigned int newTime2 = 0
    ) : vertex(vertex), route(route), position(position), routeCost(routeCost), finalTimeCost(finalTimeCost),
        newRD(newRD), newTime(newTime), newRD2(newRD2), newTime2(newTime2) {}
};

Solution *Grasp::constructSolution() {
    mt19937 generator((random_device()) ());
    uniform_int_distribution<unsigned int> distClients(1, instance.nClients());

    // start solution with one route with two clients
    vector<vector<unsigned int> > routes;
    vector<unsigned int> routeRD; // release date of a route
    vector<unsigned int> routeTime; // time to perform route
    vector<unsigned int> routeStart; // time which the route starts
    unsigned int a = distClients(generator), b;
    do b = distClients(generator); while (b == a);
    routes.push_back({0, a, b, 0});
    routeRD.push_back(max(RD[a], RD[b]));
    routeStart.push_back(routeRD.back());
    routeTime.push_back(W[0][a] + W[a][b] + W[b][0]);

    set<unsigned int> remainingClients;
    for (unsigned int i = 1; i < instance.nVertex(); i++) {
        if (i != a && i != b)
            remainingClients.insert(i);
    }

    while (!remainingClients.empty()) {
        // estimate total number of insertions to allocate necessary space
        const unsigned int totalInsertions = (remainingClients.size() + 1) *
                                             (instance.nClients() - remainingClients.size() + routes.size());
        vector<Insertion *> insertions;
        insertions.reserve(totalInsertions);

        for (unsigned int r = 0; r < routes.size(); r++) {
            const vector<unsigned int> &route = routes[r];
            for (unsigned int i = 0; i < routes[r].size() - 1; i++) { // in each arc
                unsigned int preRD = routeRD[r];
                unsigned int preTime = routeTime[r] - W[route[i]][route[i + 1]];
                for (const unsigned int &v: remainingClients) { // try to insert each client
                    unsigned int thisRD = max(preRD, RD[v]); // new release date of route
                    unsigned int thisTime = preTime + W[route[i]][v] + W[v][route[i + 1]]; // new time of route

                    // starting time of route
                    unsigned int time = r == 0 ? thisRD : max(thisRD, routeStart[r - 1] + routeTime[r - 1]);
                    time += thisTime; // ending time of route
                    const int routeCost = (int) time - (int) (routeStart[r] + routeTime[r]); // of current route

                    for (unsigned int x = r + 1; x < routes.size(); x++) {
                        time = max(time, routeRD[x]); // starting time of route x
                        time += routeTime[x]; // ending time of route x
                    }

                    const int finalTimeCost = (int) time - (int) (routeStart.back() + routeTime.back());
                    insertions.push_back(new Insertion(v, r, i, routeCost, finalTimeCost, thisRD, thisTime));

                }
            }
        }

        for (unsigned int r = 0; r < routes.size(); r++) {
            const vector<unsigned int> &route = routes[r];

            vector<unsigned int> totalTimeForward(route.size()); // total time of going from the depot to the i-th clt
            vector<unsigned int> maxRDForward(route.size()); // higher RD between all from the depot to the i-th element
            totalTimeForward[0] = W[0][route[0]];
            maxRDForward[0] = RD[route[0]];
            for (unsigned int i = 1; i < route.size(); i++) {
                totalTimeForward[i] = totalTimeForward[i - 1] + W[route[i - 1]][route[i]];
                maxRDForward[i] = max(RD[route[i]], maxRDForward[i - 1]);
            }

            vector<unsigned int> totalTimeBack(route.size()); // total time of going from the i-th element to the depot
            vector<unsigned int> maxRDBack(route.size()); // max RD between all element from i to the end
            totalTimeBack.back() = W[route.back()][0];
            maxRDBack.back() = RD[route.back()];
            for (int i = (int) route.size() - 2; i >= 0; i--) {
                totalTimeBack[i] = totalTimeBack[i + 1] + W[route[i]][route[i + 1]];
                maxRDBack[i] = max(RD[route[i]], maxRDBack[i + 1]);
            }

            // try to split the current route (F, L) in the routes (F, i) (i+1, L)
            for (unsigned int i = F(route); i < L(route); i++) { // try to insert depot at each arc
                const unsigned int rd1 = maxRDForward[i];
                const unsigned int rd2 = maxRDBack[i + 1];
                const unsigned int time1 = totalTimeForward[i] + W[route[i]][0];
                const unsigned int time2 = W[0][route[i + 1]] + totalTimeBack[i + 1];

                unsigned int time = r == 0 ? 0 : routeStart[r - 1] + routeTime[r - 1]; // ending time of previous route
                time = max(time, rd1) + time1; // ending time of first route
                time = max(time, rd2) + time2; // ending time of second route
                const int routeCost = (int) time - (int) (routeStart[r] + routeTime[r]);

                for (unsigned int x = r + 1; x < routes.size(); x++) {
                    time = max(time, routeRD[x]); // starting time of route x
                    time += routeTime[x]; // ending time of route x
                }
                const int finalTimeCost = (int) time - (int) (routeStart.back() + routeTime.back());

                insertions.push_back(new Insertion(0, r, i, routeCost, finalTimeCost, rd1, time1, rd2, time2));
            }
        }

        // sort insertions per cost
        sort(insertions.begin(), insertions.end(), [](const Insertion *a, const Insertion *b) {
            if (a->finalTimeCost != b->finalTimeCost) {
                return a->finalTimeCost < b->finalTimeCost;
            } else {
                return a->routeCost < b->routeCost;
            }
        });

        uniform_int_distribution<unsigned int> dist(0, (int) alpha * (insertions.size() - 1));
        Insertion *sel = insertions[dist(generator)];

        if (sel->vertex == 0) { // depot insertion
            // move 1 element more in the beginning to change to the depot
            routes.emplace(routes.begin() + sel->route + 1,
                           make_move_iterator(routes[sel->route].begin() + sel->position),
                           make_move_iterator(routes[sel->route].end()));
            routes[sel->route][sel->position] = routes[sel->route + 1][0]; // restore moved element
            routes[sel->route + 1][0] = 0; // change moved element to depot
            routes[sel->route][sel->position + 1] = 0; // end depot
            routes[sel->route].resize(sel->position + 2);

            // update data on routes
            routeRD[sel->route] = sel->newRD;
            routeRD.insert(routeRD.begin() + sel->route + 1, sel->newRD2);
            routeTime[sel->route] = sel->newTime;
            routeTime.insert(routeTime.begin() + sel->route + 1, sel->newTime2);
            routeStart.push_back(0); // only increase the size to update after
        } else { // client insertion
            routes[sel->route].insert(routes[sel->route].begin() + sel->position + 1, sel->vertex);
            routeRD[sel->route] = sel->newRD;
            routeTime[sel->route] = sel->newTime;
        }

        // update starting times of routes
        for (unsigned int r = sel->route; r < routes.size(); r++) {
            routeStart[r] = r == 0 ? RD[r] : max(RD[r], routeStart[r - 1] + routeTime[r - 1]);
        }

        remainingClients.erase(sel->vertex);
        for (auto &x : insertions) delete x;
        insertions.clear();
    }

    auto *s = new Solution(instance, routes);
    return s;
}
