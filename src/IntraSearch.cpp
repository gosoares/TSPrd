#include "IntraSearch.h"

#include <cassert>
#include <iostream>

#include "Rng.h"
#include "Split.h"

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route
#define N_INTRA_SEARCHES 6

IntraSearch::IntraSearch(Data& data) : data(data), searchOrder(N_INTRA_SEARCHES) {
    iota(searchOrder.begin(), searchOrder.end(), 1);
}

int IntraSearch::search(Solution* solution) {
    int oldTime = solution->time;
    shuffleSearchOrder();

    for (auto& route : solution->routes) {
        int whichSearch = 0;

        while (whichSearch < searchOrder.size()) {
            int gain = callIntraSearch(route, searchOrder[whichSearch]);

            if (gain > 0) {  // reset search order
                shuffleSearchOrder();
                whichSearch = 0;
            } else {
                whichSearch++;
            }
        }
    }

    int newTime = solution->update();
    return oldTime - newTime;
}

void IntraSearch::shuffleSearchOrder() { shuffle(searchOrder.begin(), searchOrder.end(), Rng::getGenerator()); }

int IntraSearch::callIntraSearch(std::vector<int>* route, int which) {
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
            std::cout << "ERROR invalid_neighbor_search_id" << std::endl;
            exit(1);
    }
}

// realiza o swap entre dois conjuntos de vértices seguidos, de tamanhos n1 e n2
int IntraSearch::swapSearch(std::vector<int>* route, int n1, int n2) {
    int bestI, bestJ;  // armazena os indices que representa o melhor swap
    int bestO = 0;     // representa a melhora ao realizar o swap acima

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
    for (int i = F(route); (i + n1 - 1) <= L(route); i++) {
        if (n1 != n2)  // so verifica os conjuntos entre os clientes anteriores se os conjuntos tiver tamanhos distindos
            // para evitar que o mesmo conjunto seja verificado duas vezes
            for (int j = 1; (j + n2 - 1) < i; j++) {
                int gain = evaluateSwap(route, j, i, n2, n1);
                if (gain > bestO) {
                    bestI = i;
                    bestJ = j;
                    bestO = gain;
                }
            }
        for (int j = (i + n1 - 1) + 1; (j + n2 - 1) <= L(route); j++) {
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
            std::swap(bestI, bestJ);
            std::swap(n1, n2);
        }

        std::vector<int> a(route->begin() + bestI, route->begin() + bestI + n1);  // primeiro conjunto
        std::vector<int> b(route->begin() + bestJ, route->begin() + bestJ + n2);  // segundo conjunto
        int diff = (int)n2 - (int)n1;

        // desloca os elementos que estão entre os conjuntos para suas posições finais
        if (diff < 0) {
            for (int i = bestI + n1; i < bestJ; i++) {
                route->at(i + diff) = route->at(i);
            }
        } else if (diff > 0) {
            for (int i = bestJ - 1; i >= bestI + n1; i--) {
                route->at(i + diff) = route->at(i);
            }
        }

        // copia o primeiro conjunto
        for (int x = 0; x < a.size(); x++) {
            route->at(bestJ + diff + x) = a[x];
        }

        // copia o segundo conjunto
        for (int x = 0; x < b.size(); x++) {
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
int IntraSearch::evaluateSwap(std::vector<int>* route, int i1, int i2, int n1, int n2) {
    assert(i1 + n1 - 1 < i2);
    assert(i2 + n2 - 1 <= route->size() - 2);

    int minus = data.timesMatrix[route->at(i1 - 1)][route->at(i1)]               // antes do primeiro conjunto
                + data.timesMatrix[route->at(i2 - 1)][route->at(i2)]             // antes do segundo conjunto
                + data.timesMatrix[route->at(i2 + n2 - 1)][route->at(i2 + n2)];  // depois do segundo conjunto;

    int plus = data.timesMatrix[route->at(i1 - 1)][route->at(i2)] +
               data.timesMatrix[route->at(i1 + n1 - 1)][route->at(i2 + n2)];

    if (i1 + n1 == i2) {  // se os conjuntos são adjacentes
        // no caso de conj adj sera criado um arc entre o ult cl do primeiro conjunto e primeiro cl do segundo
        plus += data.timesMatrix[route->at(i2 + n2 - 1)][route->at(i1)];
    } else {
        // quando os dois conjuntos são adjacentes os arco depois do primeiro conjunto e equivalente ao arco
        // antes do segundo conjunto, por isso so adicionamos o arco depois do primeiro conjunto no caso em que
        // os conjuntos não são adjacentes, para que não seja contado 2 vezes o seu peso
        minus += data.timesMatrix[route->at(i1 + n1 - 1)][route->at(i1 + n1)];  // depois do primeiro conjunto

        plus += data.timesMatrix[route->at(i2 - 1)][route->at(i1)] +
                data.timesMatrix[route->at(i2 + n2 - 1)][route->at(i1 + n1)];
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
int IntraSearch::reinsertionSearch(std::vector<int>* route, int n) {
    int bestI, bestJ;
    int bestGain = 0;

    for (int i = 1; i + n - 1 <= L(route); i++) {
        int minusFixed = (int)data.timesMatrix[route->at(i - 1)][route->at(i)] +
                         (int)data.timesMatrix[route->at(i + n - 1)][route->at(i + n)];
        int plusFixed = (int)data.timesMatrix[route->at(i - 1)][route->at(i + n)];

        for (int j = 0; j <= L(route); j++) {
            if (j >= i - 1 && j <= i + n - 1) continue;

            int minus = minusFixed + (int)data.timesMatrix[route->at(j)][route->at(j + 1)];
            int plus = plusFixed + (int)data.timesMatrix[route->at(j)][route->at(i)] +
                       (int)data.timesMatrix[route->at(i + n - 1)][route->at(j + 1)];

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
int IntraSearch::twoOptSearch(std::vector<int>* route) {
    int bestI, bestJ;
    int bestGain = 0;

    for (int i = 1; i <= L(route) - 1; i++) {
        int minus = (int)data.timesMatrix[route->at(i - 1)][route->at(i)] +
                    (int)data.timesMatrix[route->at(i)][route->at(i + 1)];
        int plus = 0;
        for (int j = i + 1; j <= L(route); j++) {
            minus += (int)data.timesMatrix[route->at(j)][route->at(j + 1)];
            plus += (int)data.timesMatrix[route->at(j)][route->at(j - 1)];

            int gain = minus - (plus + (int)data.timesMatrix[route->at(i - 1)][route->at(j)] +
                                (int)data.timesMatrix[route->at(i)][route->at(j + 1)]);

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
