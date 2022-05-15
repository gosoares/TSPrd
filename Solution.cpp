#include <limits>
#include <algorithm>
#include <iostream>
#include "Solution.h"
#include "Split.h"

using namespace std;

Solution::Solution(const Instance *instance) : instance(instance) {
    if (instance != nullptr) this->N = instance->nClients();
    time = 0;
}

Solution::Solution(
        const Instance &instance, vector<vector<unsigned int> *> routes
) : instance(&instance), routes(move(routes)), N(instance.nClients()) {
    time = update();
}

Solution::Solution(
        const Instance &instance, Sequence &sequence, set<unsigned int> *depotVisits
) : instance(&instance), N(sequence.size()) {
    set<unsigned int> depotVisitsTmp; // clients at the end of each route
    if (depotVisits == nullptr) {
        depotVisits = &depotVisitsTmp;
        Split::split(depotVisitsTmp, instance.getW(), instance.getRD(), sequence);
        // the split function return the time of the best routes and insert the last element of each route
        // in the depotVisits set
    }

    // create the routes given the depotVisits set
    routes.push_back(new vector<unsigned int>({0}));
    for (unsigned int i : sequence) {
        routes.back()->push_back(i);

        const bool is_in = depotVisits->find(i) != depotVisits->end();
        if (is_in) {
            routes.back()->push_back(0);
            routes.push_back(new vector<unsigned int>({0}));
        }
    }
    routes.back()->push_back(0);

    time = update(); // calculate the times
}

unsigned int Solution::update() {
    routeRD.resize(routes.size());
    routeTime.resize(routes.size());

    for (unsigned int r = 0; r < routes.size(); r++) {
        auto &route = routes[r];
        routeRD[r] = 0;
        routeTime[r] = 0;

        for (unsigned int i = 1; i < route->size(); i++) {
            // calculate time to perform route
            routeTime[r] += instance->time(route->at(i - 1), route->at(i));

            // and verify the maximum release date of the route
            unsigned int rdi = instance->releaseDateOf(route->at(i));
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

// verify if r is a empty route, and if so, delete it from routes
// return whether had a empty route
bool Solution::removeEmptyRoutes() {
    bool hasEmpty = false;
    for (int r = (int) routes.size() - 1; r >= 0; r--) {
        if (routes[r]->size() == 2) { // just the depot at start and end
            delete routes[r];
            routes.erase(routes.begin() + r);
            routeRD.erase(routeRD.begin() + r);
            routeTime.erase(routeTime.begin() + r);
            routeStart.erase(routeStart.begin() + r);
            hasEmpty = true;
        }
    }
    return hasEmpty;
}

Solution *Solution::copy() const {
    auto sol = new Solution(instance);
    sol->routes.reserve(this->routes.size());
    for (auto *route : this->routes) {
        sol->routes.push_back(new vector<unsigned int>(*route));
    }
    sol->routeRD = this->routeRD;
    sol->routeTime = this->routeTime;
    sol->routeStart = this->routeStart;
    sol->time = this->time;
    return sol;
}

void Solution::mirror(Solution *s) {
    for (auto *r: this->routes) delete r;
    this->routes.clear();
    this->routes.reserve(s->routes.size());
    for(auto *r: s->routes) {
        this->routes.push_back(new vector<unsigned int>(*r));
    }
    this->routeRD = s->routeRD;
    this->routeTime = s->routeTime;
    this->routeStart = s->routeStart;
    this->time = s->time;
    this->id = s->id;
    this->N = s->N;
}

Sequence *Solution::toSequence() const {
    auto *s = new Sequence(this->N);
    int i = 0;
    for (const vector<unsigned int> *route: routes) {
        for (unsigned int j = 1; j < route->size() - 1; j++) {
            s->at(i) = route->at(j);
            i++;
        }
    }
    return s;
}

void Solution::printRoutes() {
    for (unsigned int i = 0; i < routes.size(); i++) {
        cout << "Route " << i + 1;
        cout << "   RD(" << routeRD[i] << ")";
        cout << "   starts at " << routeStart[i];
        cout << "   ends at " << routeStart[i] + routeTime[i] << endl;

        cout << routes[i]->at(0);
        for (unsigned int j = 1; j < routes[i]->size(); j++) {
            cout << " -> " << routes[i]->at(j);
        }
        cout << endl;
    }
}

void printError(const string &error) {
    cout << "ERROR " << error << endl;
    exit(1);
}

void Solution::validate() {
    // check that all the routes are non-empty and start and end at the depot
    for (auto &route: routes) {
        if (route->size() == 2) {
            printError("found_empty_route");
        }

        if (route->front() != 0) {
            printError("route_not_starting_at_depot");
        }

        if (route->back() != 0) {
            printError("route_not_ending_at_depot");
        }
    }

    // check that clients are visited at most one time
    vector<bool> visited(instance->nVertex(), false);
    for (auto &route: routes) {
        for (unsigned int i = 1; i < route->size() - 1; i++) {
            if (visited[route->at(i)])
                printError("client_visited_more_than_once");
            visited[route->at(i)] = true;
        }
    }

    // check there's no depot in the middle of routes
    if (visited[0]) printError("depot_in_midle_of_route");

    // check that clients are visited once
    for (unsigned int v = 1; v < visited.size() - 1; v++) {
        if (!visited[v]) printError("client_not_visited");
    }

    // check all routes release date
    for (unsigned int r = 0; r < routes.size(); r++) {
        unsigned int rd = 0;
        for (unsigned int i = 1; i < routes[r]->size(); i++) {
            rd = max(rd, instance->releaseDateOf(routes[r]->at(i)));
        }
        if (routeRD[r] != rd) {
            printError("route_with_incorrect_release_date");
        }
    }

    // check all routes times
    for (unsigned int r = 0; r < routes.size(); r++) {
        unsigned int rtime = 0;
        for (unsigned int i = 1; i < routes[r]->size(); i++) {
            rtime += instance->time(routes[r]->at(i - 1), routes[r]->at(i));
        }
        if (routeTime[r] != rtime) {
            printError("route_with_incorrect_time");
        }
    }

    // check all routes starting times
    for (unsigned int r = 0; r < routes.size(); r++) {
        unsigned int start = r == 0 ? routeRD[r] : max(routeRD[r], routeStart[r - 1] + routeTime[r - 1]);
        if (routeStart[r] != start) {
            printError("route_with_incorrect_starting_time");
        }
    }

    // check completion time
    if (time != routeStart.back() + routeTime.back()) {
        printError("incorrect_solution_time");
    }
}

bool Solution::equals(Solution *other) const {
    if (this->time != other->time || this->routes.size() != other->routes.size())
        return false;

    for (unsigned int r = 0; r < this->routes.size(); r++) {
        if (this->routes[r]->size() != other->routes[r]->size()
            || this->routeRD[r] != other->routeRD[r]
            || this->routeTime[r] != other->routeTime[r])
            return false;

        for (unsigned int c = 1; c < this->routes[r]->size() - 1; c++) {
            if (this->routes[r]->at(c) != other->routes[r]->at(c))
                return false;
        }
    }
    return true;
}

// given a set of sequences, create a solution from each sequence
vector<Solution *> *Solution::solutionsFromSequences(
        const Instance &instance, vector<Sequence *> *sequences
) {
    auto *solutions = new vector<Solution *>(sequences->size());
    for (unsigned int i = 0; i < solutions->size(); i++) {
        solutions->at(i) = new Solution(instance, *(sequences->at(i)));
    }
    return solutions;
}

Solution::~Solution() {
    for (auto r : routes) {
        delete r;
    }
    routes.clear();
}
