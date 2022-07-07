#include "Solution.h"

#include "Split.h"

Solution::Solution(Data* data) : data(data) {  // TODO check this constructor usage
    if (data != nullptr) this->N = data->N;
    time = 0;
}

Solution::Solution(Data& data, std::vector<std::vector<int>*> routes) : data(&data), routes(move(routes)), N(data.N) {
    time = update();
}

Solution::Solution(Data& data, Sequence& sequence, std::set<int>* depotVisits) : data(&data), N(sequence.size()) {
    std::set<int> depotVisitsTmp;  // clients at the end of each route
    if (depotVisits == nullptr) {
        depotVisits = &depotVisitsTmp;
        Split::split(depotVisitsTmp, data.timesMatrix, data.releaseDates, sequence);
        // the split function return the time of the best routes and insert the last element of each route
        // in the depotVisits set
    }

    // create the routes given the depotVisits set
    routes.push_back(new std::vector<int>({0}));
    for (int i : sequence) {
        routes.back()->push_back(i);

        const bool is_in = depotVisits->find(i) != depotVisits->end();
        if (is_in) {
            routes.back()->push_back(0);
            routes.push_back(new std::vector<int>({0}));
        }
    }
    routes.back()->push_back(0);

    time = update();  // calculate the times
}

int Solution::update() {
    routeRD.resize(routes.size());
    routeTime.resize(routes.size());

    for (int r = 0; r < routes.size(); r++) {
        auto& route = routes[r];
        routeRD[r] = 0;
        routeTime[r] = 0;

        for (int i = 1; i < route->size(); i++) {
            // calculate time to perform route
            routeTime[r] += data->timesMatrix[route->at(i - 1)][route->at(i)];

            // and verify the maximum release date of the route
            int rdi = data->releaseDates[route->at(i)];
            if (rdi > routeRD[r]) {
                routeRD[r] = rdi;
            }
        }
    }

    return updateStartingTimes();
}

// must be called when changes are made to the release date and times of the routes
int Solution::updateStartingTimes(int from) {
    routeStart.resize(routes.size());

    for (int r = from; r < routes.size(); r++) {
        // calculate the starting time of route = max between release time and finishing time of the previous route
        // the first route always have the release time as starting time
        routeStart[r] = r == 0 ? routeRD[r] : std::max(routeRD[r], routeStart[r - 1] + routeTime[r - 1]);  //
    }
    this->time = routeStart.back() + routeTime.back();
    return time;
}

// verify if r is an empty route, and if so, delete it from routes
// return whether had an empty route
bool Solution::removeEmptyRoutes() {
    bool hasEmpty = false;
    for (int r = (int)routes.size() - 1; r >= 0; r--) {
        if (routes[r]->size() == 2) {  // just the depot at start and end
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

Solution* Solution::copy() const {
    auto sol = new Solution(data);
    sol->routes.reserve(this->routes.size());
    for (auto* route : this->routes) {
        sol->routes.push_back(new std::vector<int>(*route));
    }
    sol->routeRD = this->routeRD;
    sol->routeTime = this->routeTime;
    sol->routeStart = this->routeStart;
    sol->time = this->time;
    return sol;
}

void Solution::mirror(Solution* s) {
    for (auto* r : this->routes) delete r;
    this->routes.clear();
    this->routes.reserve(s->routes.size());
    for (auto* r : s->routes) {
        this->routes.push_back(new std::vector<int>(*r));
    }
    this->routeRD = s->routeRD;
    this->routeTime = s->routeTime;
    this->routeStart = s->routeStart;
    this->time = s->time;
    this->id = s->id;
    this->N = s->N;
}

Sequence* Solution::toSequence() const {
    Sequence* s = new Sequence(this->N);
    int i = 0;
    for (const std::vector<int>* route : routes) {
        for (int j = 1; j < route->size() - 1; j++) {
            s->at(i) = route->at(j);
            i++;
        }
    }
    return s;
}

void Solution::printRoutes() {
    for (int i = 0; i < routes.size(); i++) {
        std::cout << "Route " << i + 1;
        std::cout << "   RD(" << routeRD[i] << ")";
        std::cout << "   starts at " << routeStart[i];
        std::cout << "   ends at " << routeStart[i] + routeTime[i] << std::endl;

        std::cout << routes[i]->at(0);
        for (int j = 1; j < routes[i]->size(); j++) {
            std::cout << " -> " << routes[i]->at(j);
        }
        std::cout << std::endl;
    }
}

void printError(const std::string& error) {
    std::cout << "ERROR " << error << std::endl;
    exit(1);
}

void Solution::validate() {
    // check that all the routes are non-empty and start and end at the depot
    for (auto& route : routes) {
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
    std::vector<bool> visited(data->V, false);
    for (auto& route : routes) {
        for (int i = 1; i < route->size() - 1; i++) {
            if (visited[route->at(i)]) printError("client_visited_more_than_once");
            visited[route->at(i)] = true;
        }
    }

    // check there's no depot in the middle of routes
    if (visited[0]) printError("depot_in_middle_of_route");

    // check that clients are visited once
    for (int v = 1; v < visited.size() - 1; v++) {
        if (!visited[v]) printError("client_not_visited");
    }

    // check all routes release date
    for (int r = 0; r < routes.size(); r++) {
        int rd = 0;
        for (int i = 1; i < routes[r]->size(); i++) {
            rd = std::max<int>(rd, data->releaseDates[routes[r]->at(i)]);
        }
        if (routeRD[r] != rd) {
            printError("route_with_incorrect_release_date");
        }
    }

    // check all routes times
    for (int r = 0; r < routes.size(); r++) {
        int rtime = 0;
        for (int i = 1; i < routes[r]->size(); i++) {
            rtime += data->timesMatrix[routes[r]->at(i - 1)][routes[r]->at(i)];
        }
        if (routeTime[r] != rtime) {
            printError("route_with_incorrect_time");
        }
    }

    // check all routes starting times
    for (int r = 0; r < routes.size(); r++) {
        int start = r == 0 ? routeRD[r] : std::max(routeRD[r], routeStart[r - 1] + routeTime[r - 1]);
        if (routeStart[r] != start) {
            printError("route_with_incorrect_starting_time");
        }
    }

    // check completion time
    if (time != routeStart.back() + routeTime.back()) {
        printError("incorrect_solution_time");
    }
}

bool Solution::equals(Solution* other) const {
    if (this->time != other->time || this->routes.size() != other->routes.size()) return false;

    for (int r = 0; r < this->routes.size(); r++) {
        if (this->routes[r]->size() != other->routes[r]->size() || this->routeRD[r] != other->routeRD[r] ||
            this->routeTime[r] != other->routeTime[r])
            return false;

        for (int c = 1; c < this->routes[r]->size() - 1; c++) {
            if (this->routes[r]->at(c) != other->routes[r]->at(c)) return false;
        }
    }
    return true;
}

// given a std::set of sequences, create a solution from each sequence
std::vector<Solution*>* Solution::solutionsFromSequences(Data& data, std::vector<Sequence*>* sequences) {
    auto* solutions = new std::vector<Solution*>(sequences->size());
    for (int i = 0; i < solutions->size(); i++) {
        solutions->at(i) = new Solution(data, *(sequences->at(i)));
    }
    return solutions;
}

Solution::~Solution() {
    for (auto r : routes) {
        delete r;
    }
    routes.clear();
}
