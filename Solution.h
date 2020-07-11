//
// Created by gabriel on 6/20/20.
//

#ifndef TSPRD_SOLUTION_H
#define TSPRD_SOLUTION_H

#include <vector>
#include <set>
#include "Instance.h"

using namespace std;

class Solution {
    const Instance& instance;
    vector<unsigned int> sequence;
    vector<vector<unsigned int> > routes;
public:
    Solution(const Instance& instance, const vector<unsigned int>& sequence, bool reduce = false);
    const vector<unsigned int>& getSequence() const;
};


#endif //TSPRD_SOLUTION_H
