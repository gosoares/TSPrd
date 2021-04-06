#include <limits>
#include <algorithm>
#include "Solution.h"

using namespace std;

unsigned int split(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &S);

Solution::Solution(vector<vector<unsigned int> > routes, unsigned int time, int N) : routes(std::move(routes)),
                                                                                     time(time), N(N) {
    if (N == -1) { // default parameter
        this->N = 0;
        for (auto &r: routes) {
            this->N += r.size() - 2; // excluding the depot at the start and end of the route
        }
    }
}

Solution::Solution(const Instance &instance, Sequence &sequence) : N(sequence.size()) {
    set<unsigned int> depotVisits; // clients at the end of each route
    // in other words, there is a depot visit after each client in 'depotVisits'
    time = split(depotVisits, instance.getW(), instance.getRD(), sequence);
    // the split function return the time of the best routes and insert the last element of each route
    // in the depotVisits set

    // create the routes given the depotVisits set
    routes.push_back(vector<unsigned int>({0}));
    for (unsigned int i : sequence) {
        routes.back().push_back(i);

        const bool is_in = depotVisits.find(i) != depotVisits.end();
        if (is_in) {
            routes.back().push_back(0);
            routes.push_back(vector<unsigned int>({0}));
        }
    }
    routes.back().push_back(0);

    //    for (int i = 0; i < routes.size(); i++) {
    //        cout << "Route " << i + 1 << ": " << routes[i][0];
    //        for (int j = 1; j < routes[i].size(); j++) {
    //            cout << " -> " << routes[i][j];
    //        }
    //        cout << endl;
    //    }
}

// given a set of sequences, create a solution from each sequence
vector<Solution *> *Solution::solutionsFromSequences(
        const Instance &instance, vector<Sequence *> *sequences
) {
    auto *solutions = new vector<Solution *>(sequences->size());
    for (int i = 0; i < solutions->size(); i++) {
        solutions->at(i) = new Solution(instance, *(sequences->at(i)));
    }
    return solutions;
}

Solution *Solution::copy() const {
    vector<vector<unsigned int> > r(this->routes);
    return new Solution(r, this->time);
}

Sequence *Solution::toSequence() const {
    auto *s = new Sequence(this->N);
    int i = 0;
    for (const vector<unsigned int> &route: routes) {
        for (int j = 1; j < route.size() - 1; j++) {
            s->at(i) = route[j];
            i++;
        }
    }
    return s;
}

unsigned int Solution::getRoutesTime(const Instance &instance, const vector<vector<unsigned int> > &routes) {
    unsigned int time = 0;
    for (const vector<unsigned int> &route : routes) {
        // get max release date of elements in this route
        unsigned int maxRD = 0;
        for (int i = 1; i < route.size() - 1; i++) {
            unsigned int rdi = instance.releaseTimeOf(route[i]);
            if (rdi > maxRD) {
                maxRD = rdi;
            }
        }

        if (time < maxRD) {
            time = maxRD; // time = horario de saida da rota
        }

        // acrescenta tempo de realizar a rota
        for (int i = 1; i < route.size(); i++) {
            time += instance.time(route[i - 1], route[i]);
        }
    }

    return time;
}

unsigned int split(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &S) {
    const unsigned int V = RD.size(), // total number of vertex, including the depot
    N = V - 1; // total number of clients (excluding the depot)

    /*
     * Calcula os maiores releases dates entre o i-esimo e o j-esimo cliente
     *
     * Tambem caculamos o tempo necessario para uma rota que sai do deposito
     * visita do i-esimo ate o j-esimo cliente, e retorna ao deposito
     */
    vector<vector<unsigned int> > biggerRD(N, vector<unsigned int>(N));
    vector<vector<unsigned int> > sumT(N, vector<unsigned int>(N));
    for (int i = 0; i < N; i++) {
        unsigned int bigger = RD[S[i]];
        unsigned int sumTimes = W[0][S[i]];

        biggerRD[i][i] = bigger;
        sumT[i][i] = sumTimes + W[S[i]][0];

        for (int j = i + 1; j < S.size(); j++) {
            int rdj = (int) RD[S[j]];
            if (rdj > bigger) {
                bigger = rdj;
            }
            biggerRD[i][j] = bigger;

            sumTimes += W[S[j - 1]][S[j]];
            sumT[i][j] = sumTimes + W[S[j]][0];
        }
    }

    vector<unsigned int> bestIn(N + 1); // armazena a origem do arco que leva ao menor tempo
    vector<unsigned int> delta(N + 1, numeric_limits<unsigned int>::max()); // valor do arco (bestIn[i], i)
    delta[0] = 0;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j <= N; j++) {
            unsigned int deltaJ = max(biggerRD[i][j - 1], delta[i]) + sumT[i][j - 1];
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