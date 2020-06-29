#ifndef TSPRD_INSTANCE_H
#define TSPRD_INSTANCE_H

#include <vector>
#include <string>

using namespace std;

class Instance {
    unsigned int N;
    vector<vector<unsigned int> > W;
    vector<unsigned int> RD;
public:
    unsigned int nClients() const;
    unsigned int releaseTimeOf(unsigned int c) const;
    unsigned int time(unsigned int i, unsigned int j) const;
    explicit Instance(const string& filename);
};


#endif //TSPRD_INSTANCE_H
