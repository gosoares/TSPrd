#include "NeighborSearch.h"
#include "Split.h"
#include <cassert>
#include <chrono>
#include <algorithm>
#include <limits>
#include <iostream>

#define F(R) 1 // index of first client in a route
#define L(R) R.size() - 2 // index of last client in a route

NeighborSearch::NeighborSearch(
        const Instance &instance, bool applySplit
) : instance(instance), W(instance.getW()), RD(instance.getRD()), applySplit(applySplit),
    generator((random_device()) ()) {}

unsigned int NeighborSearch::intraSearch(Solution *solution, bool all) {
    unsigned int N = 6; // number of intra searchs algorithms
    vector<unsigned int> searchOrder(N);
    iota(searchOrder.begin(), searchOrder.end(), 1);
    shuffle(searchOrder.begin(), searchOrder.end(), generator);

    for (int r = (int) solution->routes.size() - 1; r >= 0; r--) {
//        if(!all && r != solution->routes.size() - 1
//        && (solution->routeStart[r] + solution->routeTime[r]) < solution->routeRD[r+1])
//            break; // improving this route do not improve the final solution
        for (unsigned int i = 0; i < searchOrder.size(); i++) {
            unsigned int gain = callIntraSearch(solution->routes[r], searchOrder[i]);

            if (gain > 0) {
                unsigned int lastMovement = searchOrder[i];
                i = -1;
                shuffle(searchOrder.begin(), searchOrder.end(), generator);
                if (searchOrder[0] == lastMovement) {
                    swap(searchOrder[0], searchOrder[searchOrder.size() - 1]);
                }
            }
        }
    }

    unsigned int oldTime = solution->time;
    unsigned int newTime = solution->update();
    return oldTime - newTime;
}

unsigned int NeighborSearch::callIntraSearch(vector<unsigned int> &route, unsigned int which) {
    switch (which) {
        case 1:
            return swapSearch(route, 1, 1);
        case 2:
            return swapSearch(route, 1, 2);
        case 3:
            return swapSearch(route, 2, 2);
        case 4:
            return reinsertionSearch(route, 1);
        case 5:
            return reinsertionSearch(route, 2);
        case 6:
            return twoOptSearch(route);
        default:
            cout << "ERROR invalid_neighbor_search_id" << endl;
            exit(1);
    }
}

unsigned int NeighborSearch::swapSearch(vector<unsigned int> &route, unsigned int n1, unsigned int n2) {
    unsigned int gain = 0, x;

    do {
        x = swapSearchIt(route, n1, n2);
        gain += x;
    } while (x > 0);

    return gain;
}

// realiza o swap entre dois conjuntos de vértices seguidos, de tamanhos n1 e n2
unsigned int NeighborSearch::swapSearchIt(vector<unsigned int> &route, unsigned int n1, unsigned int n2) {
    unsigned int bestI, bestJ; // armazena os indices que representa o melhor swap
    int bestO = 0; // representa a melhora ao realizar o swap acima

    /*
     * i: indice do primeiro elemento do primeiro conjunto
     * j: indice do segundo elemento do terceiro conjunto
     * n1: tamanho do primeiro elemento
     * n2: tamanho do segundo elemento
     * i + n1 - 1: indice do ultimo elemento do primeiro conjunto
     * i + n1: elemento logo após o primeiro conjunto
     * j + n2 - 1: indice do ultimo elemento do segundo conjunto
     * j + n2: elemento logo após o segundo conjunto
     *
     * a rota comeca com o elemento 0 e termina tambem com o elemento 0, que
     * representam a saida e a chegada ao deposito, por isso o primeiro e ultimo
     * elemento da rota não pode ser trocado, e portanto:
     * 1: indice do primeiro cliente visitado na rota
     * route.size() - 2: indice do ultimo cliente visitado na rota
     *
     */
    for (unsigned int i = F(route); (i + n1 - 1) <= L(route); i++) {
        if (n1 != n2) // so verifica os conjuntos entre os clientes anteriores se os conjuntos tiver tamanhos distindos
            // para evitar que o mesmo conjunto seja verificado duas vezes
            for (unsigned int j = 1; (j + n2 - 1) < i; j++) {
                int gain = verifySwap(route, j, i, n2, n1);
                if (gain > bestO) {
                    bestI = i;
                    bestJ = j;
                    bestO = gain;
                }
            }
        for (unsigned int j = (i + n1 - 1) + 1; (j + n2 - 1) <= L(route); j++) {
            int gain = verifySwap(route, i, j, n1, n2);
            if (gain > bestO) {
                bestI = i;
                bestJ = j;
                bestO = gain;
            }
        }
    }

    if (bestO > 0) { // se ha uma melhora possivel, realiza o swap
        if (bestI > bestJ) {
            swap(bestI, bestJ);
            swap(n1, n2);
        }

        vector<unsigned int> a(route.begin() + bestI, route.begin() + bestI + n1); // primeiro conjunto
        vector<unsigned int> b(route.begin() + bestJ, route.begin() + bestJ + n2); // segundo conjunto
        int diff = (int) n2 - (int) n1;

        // desloca os elementos que estão entre os conjuntos para suas posições finais
        if (diff < 0) {
            for (unsigned int i = bestI + n1; i < bestJ; i++) {
                route[i + diff] = route[i];
            }
        } else if (diff > 0) {
            for (unsigned int i = bestJ - 1; i >= bestI + n1; i--) {
                route[i + diff] = route[i];
            }
        }

        // copia o primeiro conjunto
        for (unsigned int x = 0; x < a.size(); x++) {
            route[bestJ + diff + x] = a[x];
        }

        // copia o segundo conjunto
        for (unsigned int x = 0; x < b.size(); x++) {
            route[bestI + x] = b[x];
        }
    }

    return bestO;
}

/*
 * verifica a melhora ao trocar os conjuntos a e b na rota
 * a: conjunto de n1 elementos que inicia em i1
 * b: conjunto de n2 elementos que inicia em i2
 *
 * pre-requisito: i1 < i2 e os conjuntos são disjuntos
 *
 * i1 + n1 - 1: ultimo elemento do primeiro conjunto
 * i2 + n2 - 1: ultimo elemento do segundo conjunto
 *
 * um retorno positivo representa uma diminuicao (melhora) no tempo de realizar a rota
 * enquando um negativo representa um aumento (piora)
 */
int NeighborSearch::verifySwap(
        vector<unsigned int> &route, unsigned int i1, unsigned int i2,
        unsigned int n1, unsigned int n2
) {
    assert(i1 + n1 - 1 < i2);
    assert(i2 + n2 - 1 <= route.size() - 2);

    unsigned int minus = W[route[i1 - 1]][route[i1]] // antes do primeiro conjunto
                         + W[route[i2 - 1]][route[i2]] // antes do segundo conjunto
                         + W[route[i2 + n2 - 1]][route[i2 + n2]]; // depois do segundo conjunto;

    unsigned int plus = W[route[i1 - 1]][route[i2]]
                        + W[route[i1 + n1 - 1]][route[i2 + n2]];


    if (i1 + n1 == i2) { // se os conjuntos são adjacentes
        // no caso de conj adj sera criado um arc entre o ult cl do primeiro conjunto e primeiro cl do segundo
        plus += W[route[i2 + n2 - 1]][route[i1]];
    } else {
        // quando os dois conjuntos são adjacentes os arco depois do primeiro conjunto e equivalente ao arco
        // antes do segundo conjunto, por isso so adicionamos o arco depois do primeiro conjunto no caso em que
        // os conjuntos não são adjacentes, para que não seja contado 2 vezes o seu peso
        minus += W[route[i1 + n1 - 1]][route[i1 + n1]]; // depois do primeiro conjunto

        plus += W[route[i2 - 1]][route[i1]]
                + W[route[i2 + n2 - 1]][route[i1 + n1]];
    }

    return (int) minus - (int) plus;
}

unsigned int NeighborSearch::reinsertionSearch(vector<unsigned int> &route, unsigned int n) {
    unsigned int gain = 0, x;

    do {
        x = reinsertionSearchIt(route, n);
        gain += x;
    } while (x > 0);

    return gain;
}

/*
 * Tenta realizar a reinsercao de um conjunto de n clientes adjacentes em todas as outras posicoes possiveis
 *
 * i: indice do primeiro elemento do conjunto
 * i + n - 1: indice do ultimo elemento do conjunto
 * L: indice do ultimo cliente na rota
 *
 * j representa onde será feita a tentativa de reinsercao
 */
unsigned int NeighborSearch::reinsertionSearchIt(vector<unsigned int> &route, unsigned int n) {

    unsigned int bestI, bestJ;
    int bestGain = 0;

    for (unsigned int i = 1; i + n - 1 <= L(route); i++) {
        int minusFixed = (int) W[route[i - 1]][route[i]]
                         + (int) W[route[i + n - 1]][route[i + n]];
        int plusFixed = (int) W[route[i - 1]][route[i + n]];

        for (unsigned int j = 0; j <= L(route); j++) {
            if (j >= i - 1 && j <= i + n - 1)
                continue;

            int minus = minusFixed
                        + (int) W[route[j]][route[j + 1]];
            int plus = plusFixed
                       + (int) W[route[j]][route[i]]
                       + (int) W[route[i + n - 1]][route[j + 1]];

            int gain = minus - plus;
            if (gain > bestGain) {
                bestI = i;
                bestJ = j;
                bestGain = gain;
            }
        }
    }

    if (bestGain > 0) { // perform reinsertion
        if (bestI > bestJ) {
            // rotate vertex backwards
            rotate(route.begin() + bestJ + 1, route.begin() + bestI, route.begin() + bestI + n);
        } else {
            // rotate vertex forward
            rotate(route.begin() + bestI, route.begin() + bestI + n, route.begin() + bestJ + 1);
        }
    }
    return bestGain;
}

unsigned int NeighborSearch::twoOptSearch(vector<unsigned int> &route) {
    unsigned int gain = 0, x;

    do {
        x = twoOptSearchIt(route);
        gain += x;
    } while (x > 0);

    return gain;
}

/*
 * tenta inverter a ordem de uma subrota que comeca no i-esimo cliente e termina no j-esimo cliente
 */
unsigned int NeighborSearch::twoOptSearchIt(vector<unsigned int> &route) {
    unsigned int bestI, bestJ;
    int bestGain = 0;

    for (unsigned int i = 1; i <= L(route) - 1; i++) {
        int minus = (int) W[route[i - 1]][route[i]]
                    + (int) W[route[i]][route[i + 1]];
        int plus = 0;
        for (unsigned int j = i + 1; j <= L(route); j++) {
            minus += W[route[j]][route[j + 1]];
            plus += W[route[j]][route[j - 1]];

            int gain = minus - (
                    plus + (int) W[route[i - 1]][route[j]] + (int) W[route[i]][route[j + 1]]);

            if (gain > bestGain) {
                bestI = i, bestJ = j;
                bestGain = gain;
            }
        }
    }

    if (bestGain > 0) // if improved, perform movement
        reverse(route.begin() + bestI, route.begin() + bestJ + 1);

    return bestGain;
}

unsigned int NeighborSearch::interSearch(Solution *solution) {
    unsigned int oldTime = solution->time;

    unsigned int N = 3; // number of inter searches algorithms
    vector<unsigned int> searchOrder(N);
    iota(searchOrder.begin(), searchOrder.end(), 1);
    shuffle(searchOrder.begin(), searchOrder.end(), generator);

    for (unsigned int i = 0; i < searchOrder.size(); i++) {
        unsigned int gain = callInterSearch(solution, searchOrder[i]);

        if (gain > 0) {
            unsigned int lastMovement = searchOrder[i];
            i = -1;
            shuffle(searchOrder.begin(), searchOrder.end(), generator);
            if (searchOrder[0] == lastMovement) {
                swap(searchOrder[0], searchOrder[searchOrder.size() - 1]);
            }
        }
    }

    unsigned int newTime = solution->update();
    return oldTime - newTime;
}

unsigned int NeighborSearch::callInterSearch(Solution *solution, int which) {
    switch (which) {
        case 1:
            return vertexRelocation(solution);
        case 2:
            return interSwap(solution);
        case 3:
            return insertDepotAndReorder(solution);
        default:
            cout << "ERROR invalid_neighbor_search_id" << endl;
            exit(1);
    }
}

// calculate the new ending time of route max(r1, r2) given that r1 and r2 changed
unsigned int NeighborSearch::calculateEndingTime(
        Solution *solution, unsigned int r1, unsigned int r2
) {
    if (r1 > r2) swap(r1, r2);
    // check ending time of route previous to r1
    unsigned int time = r1 == 0 ? 0 : solution->routeStart[r1 - 1] + solution->routeTime[r1 - 1];

    for (unsigned int i = r1; i <= r2; i++) {
        time = max(time, solution->routeRD[i]); // route starting time
        time += solution->routeTime[i]; // route ending time
    }
    return time;
}

// calculate the release date of the route 'r' in solution when removing 'vertex'
unsigned int NeighborSearch::routeReleaseDateRemoving(
        Solution *s, unsigned int r, unsigned int vertex
) {
    unsigned int rd = s->routeRD[r];
    if (RD[vertex] == rd) { // possibly removing the vertex with bigger RD in the route
        rd = 0;
        vector<unsigned int> &route = s->routes[r];
        for (unsigned int j = F(route); j <= L(route); j++) {
            if (route[j] == vertex) continue;
            unsigned int rdj = RD[route[j]];
            if (rdj > rd)
                rd = rdj;
        }
    }

    return rd;
}

/*
 * Calculate the ending time gain in the ending time of max(r1, r2)
 * given that the (release time, route time) ou routes r1 and r1
 * have changed to (r1RD, r1Time) and (r2RD, r2Time) respectively
 *
 * if the ending time is improved, change the given values in the solution,
 * if not, keep the original values
 */
unsigned int NeighborSearch::verifySolutionChangingRoutes(
        Solution *solution, unsigned int r1, unsigned int r2,
        unsigned int r1RD, unsigned int r1Time, unsigned int r2RD, unsigned int r2Time
) {
    // calculate the ending time of the bigger route in (r1, r2)
    unsigned int br = max(r1, r2);
    unsigned int originalTime = solution->routeStart[br] + solution->routeTime[br];

    // calculate the solution time, given the changes
    swap(r1RD, solution->routeRD[r1]);
    swap(r1Time, solution->routeTime[r1]);
    swap(r2RD, solution->routeRD[r2]);
    swap(r2Time, solution->routeTime[r2]);
    unsigned int newTime = calculateEndingTime(solution, r1, r2);

    if (newTime >= originalTime) { // if not improved, put the values back
        swap(r1RD, solution->routeRD[r1]);
        swap(r1Time, solution->routeTime[r1]);
        swap(r2RD, solution->routeRD[r2]);
        swap(r2Time, solution->routeTime[r2]);

        return 0;
    }

    return originalTime - newTime;
}

vector<pair<unsigned int, unsigned int> > getRoutesPairSequence(unsigned int nRoutes) {
    vector<pair<unsigned int, unsigned int> > sequence(nRoutes * nRoutes);
    sequence.resize(0); // resize but keep allocated space
    for (unsigned int i = 0; i < nRoutes; i++) {
        for (unsigned int j = i + 1; j < nRoutes; j++) {
            sequence.emplace_back(i, j);
        }
    }
    shuffle(sequence.begin(), sequence.end(),
            default_random_engine(chrono::system_clock::now().time_since_epoch().count()));
    return sequence;
}

unsigned int NeighborSearch::vertexRelocation(Solution *solution) {
    const unsigned int originalTime = solution->time;
    unsigned int gain;
    do {
        gain = 0;
        for (auto &routePair: getRoutesPairSequence(solution->routes.size())) {
            auto &r1 = routePair.first;
            auto &r2 = routePair.second;
            unsigned int gainIt;
            do {
                gainIt = vertexRelocationIt(solution, r1, r2);
                gainIt += vertexRelocationIt(solution, r2, r1);
                gain += gainIt;
            } while (gainIt > 0);
        }
    } while (gain > 0);
    solution->removeEmptyRoutes();
    return originalTime - solution->time;
}

unsigned int NeighborSearch::vertexRelocationIt(Solution *solution, unsigned int r1, unsigned int r2) {
    vector<unsigned int> &route1 = solution->routes[r1];
    vector<unsigned int> &route2 = solution->routes[r2];

    // try to remove a vertex from r2 and put in r1
    for (unsigned int i = F(route2); i <= L(route2); i++) {
        unsigned int vertex = route2[i];

        // check the new release date of route2 when removing 'vertex'
        unsigned int r2RD = routeReleaseDateRemoving(solution, r2, vertex);

        // calculate the new route time of route2 when removing vertex
        unsigned int r2Time = solution->routeTime[r2]
                              - W[route2[i - 1]][route2[i]] - W[route2[i]][route2[i + 1]]
                              + W[route2[i - 1]][route2[i + 1]];

        // check release date of route1, when inserting 'vertex'
        unsigned int r1RD = max(solution->routeRD[r1], RD[vertex]);

        // check where to put vertex to have the smaller route time
        unsigned int r1Time = numeric_limits<unsigned int>::max();
        unsigned int bestJ;
        for (unsigned int j = 0; j < route1.size() - 1; j++) {
            unsigned int time = solution->routeTime[r1]
                                - W[route1[j]][route1[j + 1]]
                                + W[route1[j]][vertex] + W[vertex][route1[j + 1]];
            if (time < r1Time) {
                r1Time = time;
                bestJ = j;
            }
        }

        unsigned int routeGain = verifySolutionChangingRoutes(solution, r1, r2, r1RD, r1Time, r2RD, r2Time);
        if (routeGain > 0) { // perform the movement
            route2.erase(route2.begin() + i); // delete i-th element
            route1.insert(route1.begin() + bestJ + 1, vertex);
            solution->updateStartingTimes(min(r1, r2));

            return routeGain;
        }
    }

    return 0;
}

unsigned int NeighborSearch::interSwap(Solution *solution) {
    const unsigned int originalTime = solution->time;
    unsigned int gain;
    do {
        gain = 0;
        for (auto &routePair: getRoutesPairSequence(solution->routes.size())) {
            unsigned int gainIt;
            do {
                gainIt = interSwapIt(solution, routePair.first, routePair.second);
                gain += gainIt;
            } while (gainIt > 0);
        }
    } while (gain > 0);
    return originalTime - solution->time;
}

unsigned int NeighborSearch::interSwapIt(Solution *solution, unsigned int r1, unsigned int r2) {
    vector<unsigned int> &route1 = solution->routes[r1];
    vector<unsigned int> &route2 = solution->routes[r2];

    // try to swap the i-th vertex from r1 with the j-th vertex from r2
    for (int i = F(route1); i <= (int) L(route1); i++) {
        const unsigned int vertex1 = route1[i];

        // check the new release date of route1 when removing 'vertex1'
        const unsigned int preR1RD = routeReleaseDateRemoving(solution, r1, vertex1);

        // time of the route without the arcs with vertex1
        const unsigned int preR1Time = solution->routeTime[r1]
                                       - W[route1[i - 1]][vertex1] - W[vertex1][route1[i + 1]];


        // check where to put vertex to have the smaller route time
        for (unsigned int j = F(route2); j <= L(route2); j++) {
            const unsigned int vertex2 = route2[j];
            const unsigned int r1RD = max(RD[vertex2], preR1RD);
            const unsigned int r1Time = preR1Time
                                        + W[route1[i - 1]][vertex2] + W[vertex2][route1[i + 1]];

            unsigned int r2RD = routeReleaseDateRemoving(solution, r2, vertex2); // removing vertex2
            r2RD = max(r2RD, RD[vertex1]); // inserting vertex1
            const unsigned int r2Time = solution->routeTime[r2]
                                        - W[route2[j - 1]][vertex2] - W[vertex2][route2[j + 1]]
                                        + W[route2[j - 1]][vertex1] + W[vertex1][route2[j + 1]];

            const unsigned int routeGain = verifySolutionChangingRoutes(solution, r1, r2, r1RD, r1Time, r2RD, r2Time);
            if (routeGain > 0) { // perform movement
                swap(route1[i], route2[j]);
                solution->updateStartingTimes(min(r1, r2));
                return routeGain;
            }

        }

    }

    return 0;
}

unsigned int NeighborSearch::insertDepotAndReorder(Solution *solution) {
    unsigned int originalTime = solution->time;
    bool improved;
    do {
        improved = insertDepotAndReorderIt(solution);
    } while (improved);

    return originalTime - solution->time;
}

// try to insert a depot in a route, and reorder the routes per release time
bool NeighborSearch::insertDepotAndReorderIt(Solution *s) {
    for (unsigned int r = 1; r < s->routes.size(); r++) {
        // if the ending time of the previous route is higher than the current route release date
        // its not possible to improve the ending time of the current route by adding a depot
        // because the starting time of the newly generated route cant be less than the current route start time
        if ((s->routeStart[r - 1] + s->routeTime[r - 1]) > s->routeRD[r]) continue;
        vector<unsigned int> &route = s->routes[r];

        unsigned int maxRD = 0;
        int iMax;
        // find the vertex with higher release date to try to insert depot only after it
        for (int i = F(route); i <= (int) L(route); i++) {
            unsigned int rdi = RD[route[i]];
            if (rdi > maxRD) {
                maxRD = rdi;
                iMax = i;
            }
        }


        vector<unsigned int> totalTimeForward(route.size()); // total time of going from the depot to the i-th element
        totalTimeForward[0] = W[0][route[0]];
        for (unsigned int i = 1; i < route.size(); i++)
            totalTimeForward[i] = totalTimeForward[i - 1] + W[route[i - 1]][route[i]];

        vector<unsigned int> totalTimeBack(route.size()); // total time of going from the i-th element to the depot
        vector<unsigned int> maxRDBack(route.size()); // max RD between all element from i to the end
        totalTimeBack.back() = W[route.back()][0];
        maxRDBack.back() = RD[route.back()];
        for (int i = (int) route.size() - 2; i >= 0; i--) {
            totalTimeBack[i] = totalTimeBack[i + 1] + W[route[i]][route[i + 1]];
            maxRDBack[i] = max(RD[route[i]], maxRDBack[i + 1]);
        }

        const unsigned int rd1 = maxRD; // the first generated route always have the release date of the original route
        // try to insert depot in each position after the vertex with higher release date
        for (unsigned int i = iMax; i < L(route); i++) {
            const unsigned int rd2 = maxRDBack[i + 1]; // release date of second route
            const unsigned int time1 = totalTimeForward[i] + W[route[i]][0]; // time of the first route
            const unsigned int time2 = W[0][route[i + 1]] + totalTimeBack[i + 1]; // time of the second route

            // check if the time improve if we change the original route r(1, N) to the routes r(i+1, N) and R(1, i)
            unsigned int time = max(s->routeStart[r - 1] + s->routeTime[r - 1], rd2); // starting time of first route
            time += time2; // ending time of the first route
            time = max(time, rd1); // starting time of the second route
            time += time1; // ending time of the second route

            if (time < s->routeStart[r] + s->routeTime[r]) {
                // if the new route end time is better then the previous route end time
                // update routes data as needed
                s->routeRD.insert(s->routeRD.begin() + r, rd2);
                s->routeTime[r] = time1;
                s->routeTime.insert(s->routeTime.begin() + r, time2);
                s->routeStart.push_back(0); // only increase the size to update after

                // update routes
                // move 1 element more in the beginning to change to the depot
                s->routes.emplace(s->routes.begin() + r,
                                  make_move_iterator(route.begin() + i), make_move_iterator(route.end()));

                s->routes[r + 1][i] = s->routes[r][0]; // restore the moved element
                s->routes[r][0] = 0; // change moved element to depot
                s->routes[r + 1][i + 1] = 0; // end depot
                s->routes[r + 1].resize(i + 2); // new route size after moving
                s->updateStartingTimes(r);

                return true;
            }
        }
    }

    return false;
}

unsigned int NeighborSearch::splitNs(Solution *solution) {
    Sequence *sequence = solution->toSequence();
    set<unsigned int> depotVisits;
    unsigned int splitTime = Split::split(depotVisits, instance.getW(), instance.getRD(), *sequence);

    unsigned int gain = 0;
    if (splitTime < solution->time) {
        gain = solution->time - splitTime;
        Solution newSolution(instance, *sequence, &depotVisits);
        solution->mirror(&newSolution);
    }
    delete sequence;
    return gain;
}

unsigned int NeighborSearch::educate(Solution *solution) {
    const unsigned int originalTime = solution->time;

    intraSearch(solution, true);
    int which = 1; // 0: intraSearch   1: interSearch
    bool splitImproved = false;
    do {
        bool improved;
        do {
            if (which == 0) {
                improved = intraSearch(solution) > 0;
            } else {
                improved = interSearch(solution) > 0;
            }
            which = 1 - which;
        } while (improved);

        if (applySplit)
            splitImproved = splitNs(solution) > 0;
    } while (splitImproved);

    return originalTime - solution->time;
}