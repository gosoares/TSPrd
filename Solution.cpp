#include <climits>
#include <algorithm>
#include "Solution.h"
#include <iostream>

using namespace std;

#pragma ide diagnostic ignored "modernize-pass-by-value"

void Solution::getDepositsVisits(const vector<vector<vector<unsigned int> > > &x, const vector<vector<vector<int> > > &y, unsigned int i,
                                 unsigned int j, unsigned int t) {
    int deposit = y[i][j][t];
    if (deposit == -1 || i == j)
        return;

    depositVisits.insert(sequence[deposit]);

    int tFirst = x[i][deposit][t]; // tempo para realizar a primeira rota
    unsigned int wSecond = max(0, ((int) instance.releaseTimeOf(sequence[deposit + 1])) - (((int) t) + tFirst));
    unsigned int startSecond = min(t + tFirst + wSecond, instance.getBiggerRD());

    getDepositsVisits(x, y, i, deposit, t);
    getDepositsVisits(x, y, deposit + 1, j, startSecond);
}

Solution::Solution(const Instance &instance, const vector<unsigned int> &sequence) : instance(instance),
                                                                                     sequence(sequence) {

    const unsigned int V = instance.nVertex(), N = instance.nClients(), L = N - 1;
    const unsigned int INF = UINT_MAX / (2 * V);

    /*
     * Calcula os maiores releases dates entre o i-esimo e o j-esimo cliente
     */
    vector<vector<unsigned int> > biggerRD(N, vector<unsigned int>(N));
    for (int i = 0; i < N; i++) {
        unsigned int bigger = instance.releaseTimeOf(sequence[i]);
        biggerRD[i][i] = bigger;

        for (int j = i + 1; j < sequence.size(); j++) {
            int rdj = (int) instance.releaseTimeOf(sequence[j]);
            if (rdj > bigger) {
                bigger = rdj;
            }

            biggerRD[i][j] = bigger;
        }
    }

    /*
     * x_ijt: menor tempo para visitar do i-ésimo até o j-ésimo cliente da sequencia saindo no tempo t.
     * y_ijt: indice em que será colocado uma visita ao deposito com relação a solução em x_ijt
     */
    vector<vector<vector<unsigned int> > > x(N, vector<vector<unsigned int> >(N,vector<unsigned int>(biggerRD[0][L] + 1, INF)));
    vector<vector<vector<int> > > y(N, vector<vector<int> >(N, vector<int>(biggerRD[0][L] + 1, -1)));
    for (unsigned int s = 0; s < N; s++) { // tamanho da sequencia = s+1

        for (unsigned int f = 0, l = f + s; l <= L; f++, l++) {
            // f: primeiro cliente da sequencla / l: ultimo cliente da sequencia

            // calculamos o tempo para sair do deposito visitar todos os clientes da sequencia, e voltar para o deposito
            unsigned int allClients = instance.time(0, sequence[f]) + instance.time(sequence[l], 0);
            for (unsigned int i = f; i < l; i++) {
                allClients += instance.time(sequence[i], sequence[i + 1]);
            }

            int biggerT = biggerRD[f][l];
            int tFinal = biggerT;
            if (s + 1 == N) {
                tFinal = (int) instance.releaseTimeOf(sequence[f]);
            }
            for (int t = (int) instance.releaseTimeOf(sequence[f]); t <= tFinal; t++) {
                x[f][l][t] = max(0, biggerT - t) // tempo de espera para iniciar rota com todos clientes
                             + allClients; // tempo realizar 1 rota visitando todos os clientes

                for (int i = (int) f; i < l; i++) { // testa visitar o deposito depois do cliente i
                    int tFirst = x[f][i][t]; // tempo para realizar a primeira rota

                    unsigned int wSecond = max(0, ((int) instance.releaseTimeOf(sequence[i + 1])) - (t + tFirst));
                    unsigned int startSecond = t + tFirst + wSecond;
                    unsigned int tSecond = x[i + 1][l][min((int) startSecond,
                                                           biggerT)]; // tempo para realizar a segunda rota

                    unsigned int time = tFirst + wSecond + tSecond;

                    if (time < x[f][l][t]) {
                        x[f][l][t] = time;
                        y[f][l][t] = i;
                    }
                }
            }

        }
    } // terminado calculo de x e y
    const unsigned int RD0 = instance.releaseTimeOf(sequence[0]);

    getDepositsVisits(x, y, 0, L, RD0);

    cout << "PD: ";
    for (auto v: depositVisits) {
        cout << v << "  ";
    }

    cout << endl;
    cout << "Result: " << (RD0 + x[0][L][RD0]) << endl;
}

const vector<unsigned int> &Solution::getSequence() const {
    return sequence;
}

const set<unsigned int> &Solution::getDepositVisits() const {
    return depositVisits;
}
