#ifndef TSPRD_INTERSEARCH_H
#define TSPRD_INTERSEARCH_H

#include "../Solution.h"

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route

class InterSearch {
   protected:
    Data& data;

    explicit InterSearch(Data& data) : data(data) {}

    std::vector<std::pair<int, int> > getRoutesPairSequence(int nRoutes) {
        std::vector<std::pair<int, int> > sequence(nRoutes * nRoutes);
        sequence.resize(0);  // resize but keep allocated space
        for (int i = 0; i < nRoutes; i++) {
            for (int j = i + 1; j < nRoutes; j++) {
                sequence.emplace_back(i, j);
            }
        }
        shuffle(sequence.begin(), sequence.end(), data.generator);
        return sequence;
    }

    // calculate the new ending time of route std::max(r1, r2) given that r1 and r2 changed
    int calculateEndingTime(Solution* solution, int r1, int r2) {
        if (r1 > r2) std::swap(r1, r2);
        // check ending time of route previous to r1
        int time = r1 == 0 ? 0 : solution->routeStart[r1 - 1] + solution->routeTime[r1 - 1];

        for (int i = r1; i <= r2; i++) {
            time = std::max(time, solution->routeRD[i]);  // route starting time
            time += solution->routeTime[i];               // route ending time
        }
        return time;
    }

    // calculate the release date of the route 'r' in solution when removing 'vertex'
    int routeReleaseDateRemoving(Solution* s, int r, int vertex) {
        int rd = s->routeRD[r];

        if (data.releaseDates[vertex] == rd) {  // possibly removing the vertex with bigger RD in the route
            rd = 0;
            std::vector<int>* route = s->routes[r];
            for (int j = F(route); j <= L(route); j++) {
                if (route->at(j) == vertex) continue;
                int rdj = data.releaseDates[route->at(j)];
                if (rdj > rd) rd = rdj;
            }
        }

        return rd;
    }

    /*
     * Calculate the ending time gain in the ending time of std::max(r1, r2)
     * given that the (release time, route time) ou routes r1 and r1
     * have changed to (r1RD, r1Time) and (r2RD, r2Time) respectively
     *
     * if the ending time is improved, change the given values in the solution,
     * if not, keep the original values
     */
    int verifySolutionChangingRoutes(Solution* solution, int r1, int r2, int r1RD, int r1Time, int r2RD, int r2Time) {
        // calculate the ending time of the bigger route in (r1, r2)
        int br = std::max(r1, r2);
        int originalTime = solution->routeStart[br] + solution->routeTime[br];

        // calculate the solution time, given the changes
        std::swap(r1RD, solution->routeRD[r1]);
        std::swap(r1Time, solution->routeTime[r1]);
        std::swap(r2RD, solution->routeRD[r2]);
        std::swap(r2Time, solution->routeTime[r2]);
        int newTime = calculateEndingTime(solution, r1, r2);

        if (newTime >= originalTime) {  // if not improved, put the values back
            std::swap(r1RD, solution->routeRD[r1]);
            std::swap(r1Time, solution->routeTime[r1]);
            std::swap(r2RD, solution->routeRD[r2]);
            std::swap(r2Time, solution->routeTime[r2]);

            return 0;
        }

        return originalTime - newTime;
    }

   public:
    virtual int search(Solution* solution) = 0;
};

#endif  // TSPRD_INTERSEARCH_H
