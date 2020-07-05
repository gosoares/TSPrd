#ifndef TSPRD_INSTANCE_H
#define TSPRD_INSTANCE_H

#include <vector>
#include <string>

using namespace std;

class Instance {
    unsigned int V;
    vector<vector<unsigned int> > W;
    vector<unsigned int> RD;
    unsigned int biggerRD;
public:
    explicit Instance(const string &filename);

    unsigned int releaseTimeOf(unsigned int c) const;

    unsigned int time(unsigned int i, unsigned int j) const;

    unsigned int nVertex() const;

    const vector<vector<unsigned int> > &getW() const;

    const vector<unsigned int> &getRD() const;
};


#endif //TSPRD_INSTANCE_H
