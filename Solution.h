#ifndef TSPRD_SOLUTION_H
#define TSPRD_SOLUTION_H

#include <vector>
#include <set>
#include "Instance.h"
#include <memory>

using namespace std;
using Sequence = vector<unsigned int>;

class Solution {
public:
    Solution(const Instance &instance, Sequence &sequence); // create a solution given the sequence, by applying the split algorithm
    Solution(vector<vector<unsigned int> > routes, unsigned int time, int N = -1); // create a solution given the routes
    vector<vector<unsigned int> > routes;
    unsigned int time;

    unsigned int id = 0; // aux field
    unsigned int N; // number of clients

    Sequence *toSequence() const;

    Solution *copy() const;

    static vector<Solution *> *solutionsFromSequences(const Instance &instance, vector<Sequence *> *sequences);

    static unsigned int getRoutesTime(const Instance &instance, const vector<vector<unsigned int> > &routes);
};


#endif //TSPRD_SOLUTION_H
