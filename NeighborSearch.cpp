#include "NeighborSearch.h"
#include <cassert>
#include <exception>
#include <chrono>
#include <random>
#include <algorithm>
#include <iostream>

#define L route.size() - 2 // indice do ultimo cliente em uma rota

NeighborSearch::NeighborSearch(const Instance &instance) : instance(instance) {}

int NeighborSearch::callIntraSearch(vector<unsigned int> &route, int which) {
    switch (which) {
        case 1:
            return swapSearch(route, 1, 1);
        case 2:
            return reinsertionSearch(route, 1);
        case 3:
            return twoOptSearch(route);
        default:
            throw invalid_argument("Invalid neighbor search id");
    }
}

int NeighborSearch::intraSearch(Solution *solution) {
    int N = 3; // number of intra searchs algorithms
    auto re = default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    vector<int> searchOrder(N);
    iota(searchOrder.begin(), searchOrder.end(), 1);
    shuffle(searchOrder.begin(), searchOrder.end(), re);

    for(vector<unsigned int> &route: solution->routes) {
        for (int i = 0; i < searchOrder.size(); i++) {
            int gain = callIntraSearch(route, searchOrder[i]);
            if(gain > 0) {
                int lastMovement = searchOrder[i];
                i = -1;
                shuffle(searchOrder.begin(), searchOrder.end(), re);
                if(searchOrder[0] == lastMovement) {
                    swap(searchOrder[0], searchOrder[searchOrder.size()-1]);
                }
            }
        }
    }

    int newTime = (int) Solution::getRoutesTime(instance, solution->routes);
    if(newTime < solution->time) {
        int gain = (int) solution->time - newTime;
        solution->time = newTime;
        return gain;
    }
}

int NeighborSearch::swapSearch(vector<unsigned int> &route, int n1, int n2) {
    int gain = 0, x;

    do {
        x = swapSearchIt(route, n1, n2);
        gain += x;
    } while (x > 0);

    return gain;
}

// realiza o swap entre dois conjuntos de vértices seguidos, de tamanhos n1 e n2
int NeighborSearch::swapSearchIt(vector<unsigned int> &route, int n1, int n2) {
    int bestI, bestJ; // armazena os indices que representa o melhor swap
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
    for (int i = 1; (i + n1 - 1) <= L; i++) {
        if (n1 != n2) // so verifica os conjuntos entre os clientes anteriores se os conjuntos tiver tamanhos distindos
            // para evitar que o mesmo conjunto seja verificado duas vezes
            for (int j = 1; (j + n2 - 1) < i; j++) {
                int gain = verifySwap(route, j, i, n2, n1);
                if (gain > bestO) {
                    bestI = i;
                    bestJ = j;
                    bestO = gain;
                }
            }
        for (int j = (i + n1 - 1) + 1; (j + n2 - 1) <= L; j++) {
            int gain = verifySwap(route, i, j, n1, n2);
            if (gain > bestO) {
                bestI = i;
                bestJ = j;
                bestO = gain;
            }
        }
    }

    if (bestI > bestJ) {
        swap(bestI, bestJ);
        swap(n1, n2);
    }

    if (bestO > 0) { // se ha uma melhora possivel, realiza o swap
        vector<int> a(route.begin() + bestI, route.begin() + bestI + n1); // primeiro conjunto
        vector<int> b(route.begin() + bestJ, route.begin() + bestJ + n2); // segundo conjunto
        int diff = n2 - n1;

        // desloca os elementos que estão entre os conjuntos para suas posições finais
        if (diff < 0) {
            for (int i = bestI + n1; i < bestJ; i++) {
                route[i + diff] = route[i];
            }
        } else if (diff > 0) {
            for (int i = bestJ - 1; i >= bestI + n1; i--) {
                route[i + diff] = route[i];
            }
        }

        // copia o primeiro conjunto
        for (int x = 0; x < a.size(); x++) {
            route[bestJ + diff + x] = a[x];
        }

        // copia o segundo conjunto
        for (int x = 0; x < b.size(); x++) {
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
int NeighborSearch::verifySwap(vector<unsigned int> &route, int i1, int i2, int n1, int n2) {
    assert(i1 + n1 - 1 < i2);
    assert(i2 + n2 - 1 <= route.size() - 2);

    unsigned int minus = instance.time(route[i1 - 1], route[i1]) // antes do primeiro conjunto
                         + instance.time(route[i2 - 1], route[i2]) // antes do segundo conjunto
                         + instance.time(route[i2 + n2 - 1], route[i2 + n2]); // depois do segundo conjunto;

    unsigned int plus = instance.time(route[i1 - 1], route[i2])
                        + instance.time(route[i1 + n1 - 1], route[i2 + n2]);


    if (i1 + n1 == i2) { // se os conjuntos são adjacentes
        // no caso de conj adj sera criado um arc entre o ult cl do primeiro conjunto e primeiro cl do segundo
        plus += instance.time(route[i2 + n2 - 1], route[i1]);
    } else {
        // quando os dois conjuntos são adjacentes os arco depois do primeiro conjunto e equivalente ao arco
        // antes do segundo conjunto, por isso so adicionamos o arco depois do primeiro conjunto no caso em que
        // os conjuntos não são adjacentes, para que não seja contado 2 vezes o seu peso
        minus += instance.time(route[i1 + n1 - 1], route[i1 + n1]); // depois do primeiro conjunto

        plus += instance.time(route[i2 - 1], route[i1])
                + instance.time(route[i2 + n2 - 1], route[i1 + n1]);
    }

    return (int) minus - (int) plus;
}

int NeighborSearch::reinsertionSearch(vector<unsigned int> &route, int n) {
    int gain = 0, x;

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
int NeighborSearch::reinsertionSearchIt(vector<unsigned int> &route, int n) {

    int bestI, bestJ, bestGain = 0;

    for (int i = 1; i + n - 1 <= L; i++) {
        unsigned int minusFixed = instance.time(route[i - 1], route[i])
                                  + instance.time(route[i + n - 1], route[i + n]);
        unsigned int plusFixed = instance.time(route[i - 1], route[i + n]);

        for (int j = 0; j <= L; j++) {
            if (j >= i - 1 && j <= i + n - 1)
                continue;

            unsigned int minus = minusFixed
                                 + instance.time(route[j], route[j + 1]);
            unsigned int plus = plusFixed
                                + instance.time(route[j], route[i])
                                + instance.time(route[i + n - 1], route[j + 1]);

            int gain = (int) minus - (int) plus;
            if (gain > bestGain) {
                bestI = i;
                bestJ = j;
                bestGain = gain;
            }
        }
    }

    if (bestGain > 0) { // houve ganho, realiza a reinsercao
        vector<int> a(route.begin() + bestI, route.begin() + bestI + n); // clientes a serem reinseridos

        if(bestI > bestJ) {
            for(int x = bestI - 1; x > bestJ; x--) { // desloca os elementos antes do conjunto para frente
                route[x+n] = route[x];
            }

            for (int x = 0; x < a.size(); x++) { // reinsere conjunto
                route[bestJ+1+x] = a[x];
            }
        } else {
            for(int x = bestI + n; x <= bestJ; x++) { // desloca elementos depois do conjunto para tras
                route[x-n] = route[x];
            }

            for(int x = 0; x < a.size(); x++) { // reinsere conjunto
                route[bestJ-n+1+x] = a[x];
            }
        }
    }
    return bestGain;
}

int NeighborSearch::twoOptSearch(vector<unsigned int> &route) {
    int gain = 0, x;

    do {
        x = twoOptSearchIt(route);
        gain += x;
    } while (x > 0);

    return gain;
}

/*
 * tenta inverter a ordem de uma subrota que comeca no i-esimo cliente e termina no j-esimo cliente
 */
int NeighborSearch::twoOptSearchIt(vector<unsigned int> &route) {
    int bestI, bestJ, bestGain = 0;

    for (int i = 1; i <= L - 1; i++) {
        int minus = (int) instance.time(route[i - 1], route[i])
                + (int) instance.time(route[i], route[i+1]);
        int plus = 0;
        for(int j = i + 1; j <= L; j++) {
            minus += (int) instance.time(route[j], route[j + 1]);
            plus += (int) instance.time(route[j], route[j - 1]);

            int gain = minus - (plus
                    + (int) instance.time(route[i-1], route[j])
                    + (int) instance.time(route[i], route[j+1]));

            if(gain > bestGain) {
                bestI = i, bestJ = j;
                bestGain = gain;
            }
        }
    }

    if (bestGain > 0) { // se houve ganho, inverte a subsequencia
        int diff = bestJ - bestI;
        for (int x = 0; x * 2 < diff; x++) {
            swap(route[bestI+x], route[bestJ-x]);
        }
    }

    return bestGain;
}

