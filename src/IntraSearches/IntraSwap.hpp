#ifndef TSPRD_INTRASWAP_H
#define TSPRD_INTRASWAP_H

#include <cassert>

#include "IntraSearch.hpp"

class IntraSwap : public IntraSearch {
   private:
    const std::vector<std::vector<int>>& timesMatrix;
    int n1, n2;

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
     * um retorno positivo representa uma diminuição (melhora) no tempo de realizar a rota
     * enquanto um negativo representa um aumento (piora)
     */
    int evaluateSwap(std::vector<int>* route, int i1, int i2, int n1, int n2) {
        assert(i1 + n1 - 1 < i2);
        assert(i2 + n2 - 1 <= route->size() - 2);

        int minus = timesMatrix[route->at(i1 - 1)][route->at(i1)]               // antes do primeiro conjunto
                    + timesMatrix[route->at(i2 - 1)][route->at(i2)]             // antes do segundo conjunto
                    + timesMatrix[route->at(i2 + n2 - 1)][route->at(i2 + n2)];  // depois do segundo conjunto;

        int plus =
            timesMatrix[route->at(i1 - 1)][route->at(i2)] + timesMatrix[route->at(i1 + n1 - 1)][route->at(i2 + n2)];

        if (i1 + n1 == i2) {  // se os conjuntos são adjacentes
            // no caso de conj adj sera criado um arc entre o ult cl do primeiro conjunto e primeiro cl do segundo
            plus += timesMatrix[route->at(i2 + n2 - 1)][route->at(i1)];
        } else {
            // quando os dois conjuntos são adjacentes os arco depois do primeiro conjunto e equivalente ao arco
            // antes do segundo conjunto, por isso so adicionamos o arco depois do primeiro conjunto no caso em que
            // os conjuntos não são adjacentes, para que não seja contado 2 vezes o seu peso
            minus += timesMatrix[route->at(i1 + n1 - 1)][route->at(i1 + n1)];  // depois do primeiro conjunto

            plus +=
                timesMatrix[route->at(i2 - 1)][route->at(i1)] + timesMatrix[route->at(i2 + n2 - 1)][route->at(i1 + n1)];
        }

        return (int)minus - (int)plus;
    }

   public:
    explicit IntraSwap(const std::vector<std::vector<int>>& timesMatrix, int n1, int n2)
        : timesMatrix(timesMatrix), n1(n1), n2(n2) {}

    // realiza o swap entre dois conjuntos de vértices seguidos, de tamanhos n1 e n2
    int search(std::vector<int>* route) {
        int bestI, bestJ;  // armazena os indices que representa o melhor swap
        int bestO = 0;     // representa a melhora ao realizar o swap acima

        /*
         * i: índice do primeiro elemento do primeiro conjunto
         * j: índice do segundo elemento do terceiro conjunto
         * n1: tamanho do primeiro elemento
         * n2: tamanho do segundo elemento
         * i + n1 - 1: índice do ultimo elemento do primeiro conjunto
         * i + n1: elemento logo após o primeiro conjunto
         * j + n2 - 1: índice do ultimo elemento do segundo conjunto
         * j + n2: elemento logo após o segundo conjunto
         *
         * a rota começa com o elemento 0 e termina também com o elemento 0, que
         * representam a saída e a chegada ao deposito, por isso o primeiro e ultimo
         * elemento da rota não pode ser trocado, e portanto:
         * 1: índice do primeiro cliente visitado na rota
         * route.size() - 2: índice do ultimo cliente visitado na rota
         *
         */
        for (int i = F(route); (i + n1 - 1) <= L(route); i++) {
            if (n1 !=
                n2)  // so verifica os conjuntos entre os clientes anteriores se os conjuntos tiver tamanhos distintos
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

        if (bestO > 0) {  // se ha uma melhora possível, realiza o swap
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
};

#endif  // TSPRD_INTRASWAP_H
