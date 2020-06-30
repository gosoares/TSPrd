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
    set<unsigned int> depositVisits;
    void getDepositsVisits(const vector<vector<vector<unsigned int> > > &x, const vector<vector<vector<int> > > &y, unsigned int i, unsigned int j, unsigned int t);
public:
    Solution(const Instance& instance, const vector<unsigned int>& sequence);
    const vector<unsigned int>& getSequence() const;
    const set<unsigned int>& getDepositVisits() const;
};


#endif //TSPRD_SOLUTION_H
