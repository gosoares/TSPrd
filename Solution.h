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
    Solution(vector<vector<unsigned int> > routes, unsigned int time, int N = -1);
    Solution(const Instance &instance, Sequence &sequence, bool reduce = false);
    vector<vector<unsigned int> > routes;
    unsigned int time;

    unsigned int id;
    unsigned int V; // numero de clientes

    Sequence *getSequence() const;

    Solution *copy() const;

    static vector<Solution *> *solutionsFromSequences(const Instance &instance, vector<Sequence *> *sequences, bool reduce = false);
    static unsigned int getRoutesTime(const Instance &instance, const vector<vector<unsigned int> > &routes);
};


#endif //TSPRD_SOLUTION_H
