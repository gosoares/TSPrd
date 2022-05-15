#ifndef TSPRD_SOLUTION_H
#define TSPRD_SOLUTION_H

#include <vector>
#include <set>
#include "Instance.h"
#include <memory>
#include <limits>
#include <algorithm>

using namespace std;
using Sequence = vector<unsigned int>;

class Solution {
private:
    const Instance *instance;
    explicit Solution(const Instance *instance);
public:
    Solution(const Instance &instance, Sequence &sequence, set<unsigned int> *depotVisits = nullptr); // create a solution given the sequence, by applying the split algorithm
    Solution(const Instance &instance, vector<vector<unsigned int> *> routes); // create a solution given the routes
    vector<vector<unsigned int>* > routes;

    vector<unsigned int> routeRD; // release date of each route
    vector<unsigned int> routeTime; // time to perform the route
    vector<unsigned int> routeStart; // starting time of each route
    unsigned int time; // completion time

    unsigned int id = 0; // aux field
    unsigned int N; // number of clients

    // should be called if the routes change to update values of RD, Time and Start
    // returns the new completion time
    unsigned int update();
    unsigned int updateStartingTimes(unsigned int from = 0);
    bool removeEmptyRoutes();

    void validate();
    void printRoutes();

    Sequence *toSequence() const;

    Solution *copy() const;

    void mirror(Solution *s);

    bool equals(Solution *solution) const;

    static Solution *INF() {
        auto *sol = new Solution(nullptr);
        sol->time = numeric_limits<unsigned int>::max();
        return sol;
    }

    static vector<Solution *> *solutionsFromSequences(const Instance &instance, vector<Sequence *> *sequences);

    ~Solution();
};


#endif //TSPRD_SOLUTION_H
