//
// Created by gabriel on 6/20/20.
//

#ifndef TSPRD_SOLUTION_H
#define TSPRD_SOLUTION_H

#include <vector>
#include "Instance.h"

using namespace std;

class Solution {
    const Instance& instance;
    vector<unsigned int> sequence;
    vector<unsigned int> depositVisits;
public:
    Solution(const Instance& instance, const vector<unsigned int>& sequence);
    const vector<unsigned int>& getSequence() const;
    const vector<unsigned int>& getDepositVisits() const;
};


#endif //TSPRD_SOLUTION_H
