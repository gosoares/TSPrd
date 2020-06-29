#include <climits>
#include <algorithm>
#include "Solution.h"
#include <iostream>

using namespace std;

#pragma ide diagnostic ignored "modernize-pass-by-value"

void getDepositsVisits(const Instance& instance, vector<unsigned int>& visits, const vector<vector<vector<unsigned int> > >& x, const vector<vector<vector<int> > >& y, unsigned int i, unsigned int j, const unsigned int t) {
    int deposit = y[i][j][t];
    if(deposit == -1)
        return;

    visits.push_back(deposit);
    getDepositsVisits(instance, visits, x, y, i, deposit, t);
    getDepositsVisits(instance, visits, x, y, deposit + 1, j, t);
}

Solution::Solution(const Instance &instance, const vector<unsigned int> &sequence) : instance(instance),
                                                                                     sequence(sequence) {

    /*
     * Dado uma sequência fisica, se um cliente tem um release date maior que o proximo
     * cliente da sequencia, então os dois clientes serão atendidos na mesma rota.
     *
     * Com essa ideia podemos agrupa-los em apenas 1 vértice representando os clientes naquela sequencia.
     *
     * Esse novo
     * */
    vector<vector<unsigned int> > mapping;
    mapping.push_back({0});
    mapping.push_back({sequence[0]});
    unsigned int bigger = instance.releaseTimeOf(sequence[0]);
    vector<int> newReleaseTime({(int) instance.releaseTimeOf(0), (int) instance.releaseTimeOf(sequence[0])});

    for (int i = 1; i < sequence.size(); i++) {
        unsigned int v = sequence[i];

        unsigned int rdi = instance.releaseTimeOf(v);
        mapping.push_back({v});
        newReleaseTime.push_back(rdi);
        if(rdi > bigger)
            bigger = rdi;
//        if (rdi > bigger) {
//            mapping.push_back({v});
//            bigger = rdi;
//            newReleaseTime.push_back(rdi);
//        } else {
//            mapping.back().push_back(v);
//        }
    }
    const unsigned int newNClients = mapping.size() - 1;

    /*
     * Calcula tempos entre os conjuntos de nós
     * Peso de chegada: peso de chegada do primeiro nó da sequência
     * Peso de saída: peso de saida do ultimo nó da sequencia, somado com os pesos da sequencia interna
     *
     * Vale notar: após visitar um cliente, só é possível voltar ao deposito ou ir ao proximo cliente da sequencia
     */
    vector<vector<unsigned int> > newGraph(mapping.size(), vector<unsigned int>(mapping.size(), UINT_MAX));

    for (int i = 1; i < mapping.size(); i++) {

        unsigned int inner = 0; // calcula tempo para percorrer os nós dessa sequencia
        for (int j = 1; j < mapping[i].size(); j++) {
            inner += instance.time(mapping[i][j - 1], mapping[i][j]);
        }

        newGraph[0][i] = instance.time(0, mapping[i][0]);
        newGraph[i][0] = inner + instance.time(mapping[i].back(), 0);

        if (i + 1 < mapping.size()) {
            newGraph[i][i + 1] = inner + instance.time(mapping[i].back(), mapping[i + 1][0]);
        }

    }


    /*
     * x_ijt: menor tempo para visitar do i-ésimo até o j-ésimo cliente da sequencia saindo no tempo t.
     * y_ijt: indice em que será colocado uma visita ao deposito com relação a solução em x_ijt
     */
    vector<vector<vector<unsigned int> > > x(newGraph.size(),vector<vector<unsigned int> >(newGraph.size(), vector<unsigned int>(bigger + 1, UINT_MAX / (2 * newGraph.size()))));
    vector<vector<vector<int> > > y(newGraph.size(), vector<vector<int> >(newGraph.size(),vector<int>(bigger + 1, -1)));
    for (unsigned int s = 0; s < newNClients; s++) { // tamanho da sequencia = s+1


        for (unsigned int f = 1, l = f + s; l < newGraph.size(); f++, l++) {
            // f: primeiro cliente da sequencla / l: ultimo cliente da sequencia

            // calculamos o tempo para sair do deposito visitar todos os clientes da sequencia, e voltar para o deposito
            unsigned int allClients = newGraph[0][f] + newGraph[l][0];
            for (unsigned int i = f; i < l; i++) {
                allClients += newGraph[i][i + 1];
            }

            int tFinal = newReleaseTime[l];
            if(s + 1 == newNClients) {
                tFinal = newReleaseTime[f];
            }
            for (int t = newReleaseTime[f]; t <= tFinal; t++) {
                x[f][l][t] = max(0, newReleaseTime[l] - t) // tempo de espera para iniciar rota com todos clientes
                             + allClients; // tempo realizar 1 rota visitando todos os clientes

                for (unsigned int i = f; i < l; i++) { // testa visitar o deposito depois do cliente i
                    int tFirst = x[f][i][t]; // tempo para realizar a primeira rota

                    unsigned int wSecond = max(0, newReleaseTime[i+1] - (t + tFirst));
                    unsigned int startSecond = t + tFirst + wSecond;
                    unsigned int tSecond = x[i + 1][l][min((int) startSecond, newReleaseTime[l])]; // tempo para realizar a segunda rota

                    unsigned int time = tFirst + wSecond + tSecond;

                    if (time < x[f][l][t]) {
                        x[f][l][t] = time;
                        y[f][l][t] = i;
                    }
                }
            }

        }
    } // terminado calculo de x e y
//    cout << "tempo: " << (x[1][newGraph.size()-1][newReleaseTime[1]] + newReleaseTime[1]) << endl;
    vector<unsigned int> visits;
    getDepositsVisits(visits, x, y, 1, newGraph.size() - 1, newReleaseTime[1]);

    depositVisits.reserve(visits.size());

    for (auto v: visits) {
        int ord = -1;
        for(int i = 0; i <= v; i++) {
            ord += mapping[i].size();
        }
        depositVisits.push_back(ord);
    }

    cout << "PD: ";
    for (auto v: visits) {
        cout << mapping[v].back() << "  ";
    }
    cout << endl;
    cout << "Result: " << (x[1][newGraph.size()-1][newReleaseTime[1]] + newReleaseTime[1]) << endl;
}

const vector<unsigned int> &Solution::getSequence() const {
    return sequence;
}

const vector<unsigned int> &Solution::getDepositVisits() const {
    return depositVisits;
}
