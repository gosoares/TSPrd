#ifndef TSPRD_SPLIT_H
#define TSPRD_SPLIT_H

#include <set>
#include <vector>

class Split {
   public:
    static int split(std::set<int>& visits, const std::vector<std::vector<int> >& W, const std::vector<int>& RD,
                     const std::vector<int>& S) {
        const int V = RD.size(),  // total number of vertex, including the depot
            N = V - 1;            // total number of clients (excluding the depot)

        // stores for each position in the sequence, which vertex before it can be the one that determines the release
        // date of the route containing the vertex on that position
        std::vector<int> rdPos(N);
        rdPos[0] = 0;

        std::vector<int> cumulative(S.size());
        cumulative[0] = 0;  // cumulative of the arcs times till vertex in position i

        // calculate the position of the vertices with increasing release dates
        for (int i = 1; i < S.size(); i++) {
            if (RD[S[i]] > RD[S[rdPos[i - 1]]]) {
                rdPos[i] = i;
            } else {
                rdPos[i] = rdPos[i - 1];
            }
            cumulative[i] = cumulative[i - 1] + W[S[i - 1]][S[i]];
        }

        std::vector<int> bestIn(N);                                // store the origin of the best arc arriving at i
        std::vector<int> phi(N, std::numeric_limits<int>::max());  // value of the best arc arriving at i

        for (int j = 0; j < N; j++) {
            int rdPosJ = rdPos[j];
            int rdj = RD[S[rdPosJ]];
            int jToDepot = W[S[j]][0];
            int cumulativeJ = cumulative[j];

            for (int i = 0; i <= rdPosJ; i++) {
                int sigma = rdj;
                if (i > 0) sigma = std::max(sigma, phi[i - 1]);

                int deltaJ = sigma + W[0][S[i]] + (cumulativeJ - cumulative[i]) + jToDepot;
                if (deltaJ < phi[j]) {
                    phi[j] = deltaJ;
                    bestIn[j] = i;
                }
            }
        }

        int x = N - 1;
        while (bestIn[x] > 0) {
            x = bestIn[x] - 1;
            visits.insert(S[x]);
        }

        return phi.back();
    }
};

#endif  // TSPRD_SPLIT_H
