#ifndef TSPRD_SOLUTION_H
#define TSPRD_SOLUTION_H

#include <vector>
#include <set>
#include "Instance.h"
#include <memory>

using namespace std;
using Sequence = vector<unsigned int>;

class Solution {
private:
    const Instance *instance;
public:
    Solution(const Instance &instance, Sequence &sequence); // create a solution given the sequence, by applying the split algorithm
    Solution(vector<vector<unsigned int> > routes, unsigned int time, const Instance *instance = nullptr); // create a solution given the routes
    vector<vector<unsigned int> > routes;

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

    void validate();
    void printRoutes();

    Sequence *toSequence() const;

    Solution *copy() const;

    static vector<Solution *> *solutionsFromSequences(const Instance &instance, vector<Sequence *> *sequences);
};


#endif //TSPRD_SOLUTION_H
