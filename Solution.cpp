#include <limits>
#include <algorithm>
#include <iostream>
#include <cassert>
#include "Solution.h"

using namespace std;

unsigned int split(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &S);

Solution::Solution(
        vector<vector<unsigned int> > routes, unsigned int time, const Instance *instance
) : routes(std::move(routes)), time(time), instance(instance) {
    this->N = 0;
    for (auto &r: routes) {
        this->N += r.size() - 2; // excluding the depot at the start and end of the route
    }
}

Solution::Solution(const Instance &instance, Sequence &sequence) : N(sequence.size()), instance(&instance) {
    set<unsigned int> depotVisits; // clients at the end of each route
    // in other words, there is a depot visit after each client in 'depotVisits'
    unsigned int splitTime = split(depotVisits, instance.getW(), instance.getRD(), sequence);
    // the split function return the time of the best routes and insert the last element of each route
    // in the depotVisits set

    // create the routes given the depotVisits set
    routes.push_back(vector<unsigned int>({0}));
    for (unsigned int i : sequence) {
        routes.back().push_back(i);

        const bool is_in = depotVisits.find(i) != depotVisits.end();
        if (is_in) {
            routes.back().push_back(0);
            routes.push_back(vector<unsigned int>({0}));
        }
    }
    routes.back().push_back(0);

    time = update(); // calculate the times
    assert(splitTime == time);
}

unsigned int Solution::update() {
    routeRD.resize(routes.size());
    routeTime.resize(routes.size());

    for (int r = 0; r < routes.size(); r++) {
        auto &route = routes[r];
        routeRD[r] = 0;
        routeTime[r] = 0;

        for (int i = 1; i < route.size(); i++) {
            // calculate time to perform route
            routeTime[r] += instance->time(route[i - 1], route[i]);

            // and verify the maximum release date of the route
            unsigned int rdi = instance->releaseDateOf(route[i]);
            if (rdi > routeRD[r]) {
                routeRD[r] = rdi;
            }
        }
    }

    return updateStartingTimes();
}

// must be called when changes are made to the release date and times of the routes
unsigned int Solution::updateStartingTimes(unsigned int from) {
    routeStart.resize(routes.size());

    for (unsigned int r = from; r < routes.size(); r++) {
        // calculate the starting time of route = max between release time and finishing time of the previous route
        // the first route always have the release time as starting time
        routeStart[r] = r == 0 ? routeRD[r] : max(routeRD[r], routeStart[r - 1] + routeTime[r - 1]); //
    }
    this->time = routeStart.back() + routeTime.back();
    return time;
}


// given a set of sequences, create a solution from each sequence
vector<Solution *> *Solution::solutionsFromSequences(
        const Instance &instance, vector<Sequence *> *sequences
) {
    auto *solutions = new vector<Solution *>(sequences->size());
    for (int i = 0; i < solutions->size(); i++) {
        solutions->at(i) = new Solution(instance, *(sequences->at(i)));
    }
    return solutions;
}

Solution *Solution::copy() const {
    vector<vector<unsigned int> > r(this->routes);
    auto sol = new Solution(r, this->time, instance);
    sol->routeRD = this->routeRD;
    sol->routeTime = this->routeTime;
    sol->routeStart = this->routeStart;
    return sol;
}

Sequence *Solution::toSequence() const {
    auto *s = new Sequence(this->N);
    int i = 0;
    for (const vector<unsigned int> &route: routes) {
        for (int j = 1; j < route.size() - 1; j++) {
            s->at(i) = route[j];
            i++;
        }
    }
    return s;
}

void Solution::printRoutes() {
    for (int i = 0; i < routes.size(); i++) {
        cout << "Route " << i + 1 << ": " << routes[i][0];
        for (int j = 1; j < routes[i].size(); j++) {
            cout << " -> " << routes[i][j];
        }
        cout << endl;
    }
}

void Solution::validate() {
    // check if all clients are visited once
    vector<bool> visited(instance->nVertex(), false);
    visited[0] = true;
    for (auto &route: routes) {
        for (unsigned int i = 1; i < route.size() - 1; i++) {
            if (visited[route[i]])
                throw logic_error("client " + to_string(i) + " visited more than once");
            visited[route[i]] = true;
        }
    }

    // check all routes release date
    for (unsigned int r = 0; r < routes.size(); r++) {
        unsigned int rd = 0;
        for (unsigned int i = 1; i < routes[r].size(); i++) {
            rd = max(rd, instance->releaseDateOf(routes[r][i]));
        }
        if (routeRD[r] != rd) {
            throw logic_error("route " + to_string(r) + " has incorrect release date");
        }
    }

    // check all routes times
    for (unsigned int r = 0; r < routes.size(); r++) {
        unsigned int rtime = 0;
        for (int i = 1; i < routes[r].size(); i++) {
            rtime += instance->time(routes[r][i - 1], routes[r][i]);
        }
        if (routeTime[r] != rtime) {
            throw logic_error("route " + to_string(r) + " has incorrect time");
        }
    }

    // check all routes starting times
    for (unsigned int r = 0; r < routes.size(); r++) {
        unsigned int start = r == 0 ? routeRD[r] : max(routeRD[r], routeStart[r - 1] + routeTime[r - 1]);
        if (routeStart[r] != start) {
            throw logic_error("route " + to_string(r) + " has incorrect starting time");
        }
    }

    // check completion time
    if (time != routeStart.back() + routeTime.back()) {
        throw logic_error("incorrect solution time");
    }
}

unsigned int split(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &S) {
    const unsigned int V = RD.size(), // total number of vertex, including the depot
    N = V - 1; // total number of clients (excluding the depot)

    /*
     * Calcula os maiores releases dates entre o i-esimo e o j-esimo cliente
     *
     * Tambem caculamos o tempo necessario para uma rota que sai do deposito
     * visita do i-esimo ate o j-esimo cliente, e retorna ao deposito
     */
    vector<vector<unsigned int> > biggerRD(N, vector<unsigned int>(N));
    vector<vector<unsigned int> > sumT(N, vector<unsigned int>(N));
    for (int i = 0; i < N; i++) {
        unsigned int bigger = RD[S[i]];
        unsigned int sumTimes = W[0][S[i]];

        biggerRD[i][i] = bigger;
        sumT[i][i] = sumTimes + W[S[i]][0];

        for (int j = i + 1; j < S.size(); j++) {
            int rdj = (int) RD[S[j]];
            if (rdj > bigger) {
                bigger = rdj;
            }
            biggerRD[i][j] = bigger;

            sumTimes += W[S[j - 1]][S[j]];
            sumT[i][j] = sumTimes + W[S[j]][0];
        }
    }

    vector<unsigned int> bestIn(N + 1); // armazena a origem do arco que leva ao menor tempo
    vector<unsigned int> delta(N + 1, numeric_limits<unsigned int>::max()); // valor do arco (bestIn[i], i)
    delta[0] = 0;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j <= N; j++) {
            unsigned int deltaJ = max(biggerRD[i][j - 1], delta[i]) + sumT[i][j - 1];
            if (deltaJ < delta[j]) {
                delta[j] = deltaJ;
                bestIn[j] = i;
            }
        }
    }

    unsigned int x = bestIn[N];
    while (x > 0) {
        visits.insert(S[x - 1]);
        x = bestIn[x];
    }

    return delta.back();
}