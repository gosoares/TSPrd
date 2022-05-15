#ifndef TSPRD_SPLIT_H
#define TSPRD_SPLIT_H

#include <set>
#include <vector>

using namespace std;

class Split {
public:
    static unsigned int split(
            set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
            const vector<unsigned int> &S
    ) {
        const unsigned int V = RD.size(), // total number of vertex, including the depot
        N = V - 1; // total number of clients (excluding the depot)


        // stores for each position in the sequence, which vertex before it can be the one that determines the release date
        // of the route containing the vertex on that position
        vector<unsigned int> rdPos(N);
        rdPos[0] = 0;

        vector<unsigned int> cumulative(S.size());
        cumulative[0] = 0; // cumulative of the arcs times till vertex in position i

        // calculate the position of the vertices with increasing release dates
        for (unsigned int i = 1; i < S.size(); i++) {
            if (RD[S[i]] > RD[S[rdPos[i - 1]]]) {
                rdPos[i] = i;
            } else {
                rdPos[i] = rdPos[i - 1];
            }
            cumulative[i] = cumulative[i - 1] + W[S[i - 1]][S[i]];
        }

        vector<unsigned int> bestIn(N); // store the origin of the best arc arriving at i
        vector<unsigned int> phi(N, numeric_limits<unsigned int>::max()); // value of the best arc arriving at i

        for (unsigned int j = 0; j < N; j++) {
            unsigned int rdPosJ = rdPos[j];
            unsigned int rdj = RD[S[rdPosJ]];
            unsigned int jToDepot = W[S[j]][0];
            unsigned int cumulativeJ = cumulative[j];

            for (unsigned int i = 0; i <= rdPosJ; i++) {
                unsigned int sigma = rdj;
                if (i > 0) sigma = max(sigma, phi[i - 1]);

                unsigned int deltaJ = sigma + W[0][S[i]] + (cumulativeJ - cumulative[i]) + jToDepot;
                if (deltaJ < phi[j]) {
                    phi[j] = deltaJ;
                    bestIn[j] = i;
                }
            }
        }

        unsigned int x = N - 1;
        while (bestIn[x] > 0) {
            x = bestIn[x] - 1;
            visits.insert(S[x]);
        }

        return phi.back();
    }
};

#endif //TSPRD_SPLIT_H
