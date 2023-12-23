#ifndef TSPRD_SOLUTION_H
#define TSPRD_SOLUTION_H

#include "Data.hpp"

using Sequence = std::vector<int>;

class Solution {
   private:
    Data* data;
    explicit Solution(Data* data);

   public:
    // create a solution given the sequence, by applying the split algorithm
    Solution(Data& data, Sequence& sequence, std::set<int>* depotVisits);
    Solution(Data& data, std::vector<std::vector<int>*> routes);  // create a solution given the routes
    std::vector<std::vector<int>*> routes;

    std::vector<int> routeRD;     // release date of each route
    std::vector<int> routeTime;   // time to perform the route
    std::vector<int> routeStart;  // starting time of each route
    int time;                     // completion time

    int id = 0;  // aux field
    int N;       // number of clients

    // should be called if the routes change to update values of RD, Time and Start
    // returns the new completion time
    int update();
    int updateStartingTimes(int from = 0);
    bool removeEmptyRoutes();

    void validate();
    void printRoutes();

    Sequence* toSequence() const;

    Solution* copy() const;

    void mirror(Solution* s);

    bool equals(Solution* solution) const;

    static Solution* worst() {
        auto* sol = new Solution(nullptr);
        sol->time = INF;
        return sol;
    }

    static std::vector<Solution*>* solutionsFromSequences(Data& data, std::vector<Sequence*>* sequences);

    static int split(std::set<int>& visits, const std::vector<std::vector<int> >& W, const std::vector<int>& RD,
                     const std::vector<int>& S);

    ~Solution();
};

#endif  // TSPRD_SOLUTION_H
