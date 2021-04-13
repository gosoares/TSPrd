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

        /*
         * Calculate the bigger release dates between the i-th and j-th clients
         * This value is the release date of the route that visits those clients
         *
         * Also calculate the time to perform a route that visits those clients in that order
         * including the times from the depot to the i-th client, and from the j-th client to the depot
         */
        vector<vector<unsigned int> > routesRD(N, vector<unsigned int>(N));
        vector<vector<unsigned int> > routesTime(N, vector<unsigned int>(N));
        for (int i = 0; i < N; i++) {
            unsigned int bigger = RD[S[i]];
            unsigned int sumTimes = W[0][S[i]];

            routesRD[i][i] = bigger;
            routesTime[i][i] = sumTimes + W[S[i]][0];

            for (int j = i + 1; j < S.size(); j++) {
                int rdj = (int) RD[S[j]];
                if (rdj > bigger) {
                    bigger = rdj;
                }
                routesRD[i][j] = bigger;

                sumTimes += W[S[j - 1]][S[j]];
                routesTime[i][j] = sumTimes + W[S[j]][0];
            }
        }

        vector<unsigned int> bestIn(N + 1); // store the origin of the best arc arriving at i
        vector<unsigned int> delta(N + 1, numeric_limits<unsigned int>::max()); // value of the best arc arriving at i
        delta[0] = 0;

        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j <= N; j++) {
                unsigned int deltaJ = max(routesRD[i][j - 1], delta[i]) + routesTime[i][j - 1];
                if (deltaJ < delta[j]) {
                    delta[j] = deltaJ;
                    bestIn[j] = i;
                }
            }
        }

        unsigned int x = bestIn[N];
        while (x > 0) {
            visits.insert(S[x - 1]);
            x = bestIn[x];
        }

        return delta.back();
    }
};

#endif //TSPRD_SPLIT_H
