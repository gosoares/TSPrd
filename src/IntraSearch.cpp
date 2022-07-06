#include "IntraSearch.h"

#include <cassert>
#include <iostream>

#include "Rng.h"
#include "Split.h"

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route
#define N_INTRA_SEARCHES 6

IntraSearch::IntraSearch(const Instance& instance) : W(instance.getW()), searchOrder(N_INTRA_SEARCHES) {
    iota(searchOrder.begin(), searchOrder.end(), 1);
}

unsigned int IntraSearch::search(Solution* solution) {
    unsigned int oldTime = solution->time;
    shuffleSearchOrder();

    for (auto& route : solution->routes) {
        unsigned int whichSearch = 0;

        while (whichSearch < searchOrder.size()) {
            unsigned int gain = callIntraSearch(route, searchOrder[whichSearch]);

            if (gain > 0) {  // reset search order
                shuffleSearchOrder();
                whichSearch = 0;
            } else {
                whichSearch++;
            }
        }
    }

    unsigned int newTime = solution->update();
    return oldTime - newTime;
}

void IntraSearch::shuffleSearchOrder() { shuffle(searchOrder.begin(), searchOrder.end(), Rng::getGenerator()); }

unsigned int IntraSearch::callIntraSearch(vector<unsigned int>* route, unsigned int which) {
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

// realiza o swap entre dois conjuntos de vértices seguidos, de tamanhos n1 e n2
unsigned int IntraSearch::swapSearch(vector<unsigned int>* route, unsigned int n1, unsigned int n2) {
    unsigned int bestI, bestJ;  // armazena os indices que representa o melhor swap
    int bestO = 0;              // representa a melhora ao realizar o swap acima

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
        if (n1 != n2)  // so verifica os conjuntos entre os clientes anteriores se os conjuntos tiver tamanhos distindos
            // para evitar que o mesmo conjunto seja verificado duas vezes
            for (unsigned int j = 1; (j + n2 - 1) < i; j++) {
                int gain = evaluateSwap(route, j, i, n2, n1);
                if (gain > bestO) {
                    bestI = i;
                    bestJ = j;
                    bestO = gain;
                }
            }
        for (unsigned int j = (i + n1 - 1) + 1; (j + n2 - 1) <= L(route); j++) {
            int gain = evaluateSwap(route, i, j, n1, n2);
            if (gain > bestO) {
                bestI = i;
                bestJ = j;
                bestO = gain;
            }
        }
    }

    if (bestO > 0) {  // se ha uma melhora possivel, realiza o swap
        if (bestI > bestJ) {
            swap(bestI, bestJ);
            swap(n1, n2);
        }

        vector<unsigned int> a(route->begin() + bestI, route->begin() + bestI + n1);  // primeiro conjunto
        vector<unsigned int> b(route->begin() + bestJ, route->begin() + bestJ + n2);  // segundo conjunto
        int diff = (int)n2 - (int)n1;

        // desloca os elementos que estão entre os conjuntos para suas posições finais
        if (diff < 0) {
            for (unsigned int i = bestI + n1; i < bestJ; i++) {
                route->at(i + diff) = route->at(i);
            }
        } else if (diff > 0) {
            for (unsigned int i = bestJ - 1; i >= bestI + n1; i--) {
                route->at(i + diff) = route->at(i);
            }
        }

        // copia o primeiro conjunto
        for (unsigned int x = 0; x < a.size(); x++) {
            route->at(bestJ + diff + x) = a[x];
        }

        // copia o segundo conjunto
        for (unsigned int x = 0; x < b.size(); x++) {
            route->at(bestI + x) = b[x];
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
int IntraSearch::evaluateSwap(vector<unsigned int>* route, unsigned int i1, unsigned int i2, unsigned int n1,
                              unsigned int n2) {
    assert(i1 + n1 - 1 < i2);
    assert(i2 + n2 - 1 <= route->size() - 2);

    unsigned int minus = W[route->at(i1 - 1)][route->at(i1)]               // antes do primeiro conjunto
                         + W[route->at(i2 - 1)][route->at(i2)]             // antes do segundo conjunto
                         + W[route->at(i2 + n2 - 1)][route->at(i2 + n2)];  // depois do segundo conjunto;

    unsigned int plus = W[route->at(i1 - 1)][route->at(i2)] + W[route->at(i1 + n1 - 1)][route->at(i2 + n2)];

    if (i1 + n1 == i2) {  // se os conjuntos são adjacentes
        // no caso de conj adj sera criado um arc entre o ult cl do primeiro conjunto e primeiro cl do segundo
        plus += W[route->at(i2 + n2 - 1)][route->at(i1)];
    } else {
        // quando os dois conjuntos são adjacentes os arco depois do primeiro conjunto e equivalente ao arco
        // antes do segundo conjunto, por isso so adicionamos o arco depois do primeiro conjunto no caso em que
        // os conjuntos não são adjacentes, para que não seja contado 2 vezes o seu peso
        minus += W[route->at(i1 + n1 - 1)][route->at(i1 + n1)];  // depois do primeiro conjunto

        plus += W[route->at(i2 - 1)][route->at(i1)] + W[route->at(i2 + n2 - 1)][route->at(i1 + n1)];
    }

    return (int)minus - (int)plus;
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
unsigned int IntraSearch::reinsertionSearch(vector<unsigned int>* route, unsigned int n) {
    unsigned int bestI, bestJ;
    int bestGain = 0;

    for (unsigned int i = 1; i + n - 1 <= L(route); i++) {
        int minusFixed = (int)W[route->at(i - 1)][route->at(i)] + (int)W[route->at(i + n - 1)][route->at(i + n)];
        int plusFixed = (int)W[route->at(i - 1)][route->at(i + n)];

        for (unsigned int j = 0; j <= L(route); j++) {
            if (j >= i - 1 && j <= i + n - 1) continue;

            int minus = minusFixed + (int)W[route->at(j)][route->at(j + 1)];
            int plus = plusFixed + (int)W[route->at(j)][route->at(i)] + (int)W[route->at(i + n - 1)][route->at(j + 1)];

            int gain = minus - plus;
            if (gain > bestGain) {
                bestI = i;
                bestJ = j;
                bestGain = gain;
            }
        }
    }

    if (bestGain > 0) {  // perform reinsertion
        if (bestI > bestJ) {
            // rotate vertex backwards
            rotate(route->begin() + bestJ + 1, route->begin() + bestI, route->begin() + bestI + n);
        } else {
            // rotate vertex forward
            rotate(route->begin() + bestI, route->begin() + bestI + n, route->begin() + bestJ + 1);
        }
    }
    return bestGain;
}

/*
 * tenta inverter a ordem de uma subrota que comeca no i-esimo cliente e termina no j-esimo cliente
 */
unsigned int IntraSearch::twoOptSearch(vector<unsigned int>* route) {
    unsigned int bestI, bestJ;
    int bestGain = 0;

    for (unsigned int i = 1; i <= L(route) - 1; i++) {
        int minus = (int)W[route->at(i - 1)][route->at(i)] + (int)W[route->at(i)][route->at(i + 1)];
        int plus = 0;
        for (unsigned int j = i + 1; j <= L(route); j++) {
            minus += (int)W[route->at(j)][route->at(j + 1)];
            plus += (int)W[route->at(j)][route->at(j - 1)];

            int gain = minus - (plus + (int)W[route->at(i - 1)][route->at(j)] + (int)W[route->at(i)][route->at(j + 1)]);

            if (gain > bestGain) {
                bestI = i, bestJ = j;
                bestGain = gain;
            }
        }
    }

    if (bestGain > 0)  // if improved, perform movement
        reverse(route->begin() + bestI, route->begin() + bestJ + 1);

    return bestGain;
}
