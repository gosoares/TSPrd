#include <climits>
#include <algorithm>
#include "Solution.h"
#include <iostream>

using namespace std;

#pragma ide diagnostic ignored "modernize-pass-by-value"

set<unsigned int> solutionFromSequence(const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD, const vector<unsigned int> &sequence);

Solution::Solution(const Instance &instance, const vector<unsigned int> &sequence) : instance(instance),
                                                                                     sequence(sequence) {
    set<unsigned int> depositVisits = solutionFromSequence(instance.getW(), instance.getRD(), sequence);

    routes.emplace_back(0);

    for (unsigned int i : sequence) {
        routes.back().push_back(i);

        const bool is_in = depositVisits.find(i) != depositVisits.end();
        if(is_in) {
            routes.emplace_back(0);
        }
    }
}

void getDepositsVisits(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD, const vector<unsigned int> &sequence, const vector<vector<vector<unsigned int> > > &x, const vector<vector<vector<int> > > &y,
                                 unsigned int i,
                                 unsigned int j, unsigned int t, unsigned int biggerRD) {
    int deposit = y[i][j][t];
    if (deposit == -1 || i == j)
        return;

    visits.insert(sequence[deposit]);

    int tFirst = x[i][deposit][t]; // tempo para realizar a primeira rota
    unsigned int wSecond = max(0, ((int) RD[sequence[deposit + 1]]) - (((int) t) + tFirst));
    unsigned int startSecond = min(t + tFirst + wSecond, biggerRD);

    getDepositsVisits(visits, W, RD, sequence, x, y, i, deposit, t, biggerRD);
    getDepositsVisits(visits, W, RD, sequence, x, y, deposit + 1, j, startSecond, biggerRD);
}

// dada uma sequencia, calcula onde colocar os depositos para se obter o menor tempo total
set<unsigned int> solutionFromSequence(const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD, const vector<unsigned int> &sequence) {
    const unsigned int V = RD.size(), N = V - 1, L = N - 1;
    const unsigned int INF = UINT_MAX / (2 * V);

    /*
     * Calcula os maiores releases dates entre o i-esimo e o j-esimo cliente
     * É precalculado o valor e o indice do elemento que tem o maior release date
     *
     * Tambem caculamos o tempo necessario para uma rota que sai do deposito
     * visita do i-esimo ate o j-esimo cliente, e retorna ao deposito
     */
    vector<vector<unsigned int> > biggerRD(N, vector<unsigned int>(N));
    vector<vector<unsigned int> > iBiggerRD(N, vector<unsigned int>(N));
    vector<vector<unsigned int> > sumT(N, vector<unsigned int>(N));
    for (int i = 0; i < N; i++) {
        unsigned int iBigger = i;
        unsigned int bigger = RD[sequence[i]];
        unsigned int sumTimes = W[0][sequence[i]];

        biggerRD[i][i] = bigger;
        iBiggerRD[i][i] = iBigger;
        sumT[i][i] = sumTimes + W[sequence[i]][0];

        for (int j = i + 1; j < sequence.size(); j++) {
            int rdj = (int) RD[sequence[j]];
            if (rdj > bigger) {
                iBigger = j;
                bigger = rdj;
            }
            iBiggerRD[i][j] = iBigger;
            biggerRD[i][j] = bigger;

            sumTimes += W[sequence[j - 1]][sequence[j]];
            sumT[i][j] = sumTimes + W[sequence[j]][0];
        }
    }

    /*
     * Agora calculamos o menor e o maior tempo que uma subsequencia pode começar em uma solução
     *
     * Para descobrir o menor e o maior tempo que uma subsequencia que começa no i-esimo elemento pode comecar,
     * precisamos saber quem é o cliente 'a' que tem maior release date entre os clientes anteriores ao i-ésimo.
     *
     * O menor tempo então que a subsequencia começando em i-ésimo começar é o maximo entre seu proprio release date e
     * RD[a] + sumT[a][i-1] que equivale a o tempo final de uma rota que sai no tempo RD[a] e vai do cliente a até o cliente [i-1]
     *
     * O maior tempo equivale a o tempo de uma rota saindo no tempo RD[a] e percorrento a rota do primeiro até o (i-1)-ésimo
     * elemento da sequencia
     */
    vector<vector<unsigned int> > minT(N, vector<unsigned int>(N));
    vector<vector<unsigned int> > maxT(N, vector<unsigned int>(N));
    for (int i = 0; i < N; i++) {
        unsigned int mint = 0;
        unsigned int biggerRDbeforei = INF;
        if (i > 0) {
            int ibiggerRDbeforei = iBiggerRD[0][i - 1];
            biggerRDbeforei = biggerRD[0][i - 1];
            mint = biggerRDbeforei; // + sumT[ibiggerRDbeforei][i-1];
        }
        mint = max(mint, RD[sequence[i]]);

        for (int j = i; j < sequence.size(); j++) {
            minT[i][j] = mint;

            unsigned int maxt = biggerRDbeforei + sumT[0][i-1];
//            maxT[i][j] = min(maxt, biggerRD[i][j]);
            maxT[i][j] = biggerRD[i][j];
        }
    }

    /*
     * x_ijt: menor tempo para visitar do i-ésimo até o j-ésimo cliente da sequencia saindo no tempo t.
     * y_ijt: indice em que será colocado uma visita ao deposito com relação a solução em x_ijt
     */
    vector<vector<vector<unsigned int> > > x(N, vector<vector<unsigned int> >(N,
                                                                              vector<unsigned int>(biggerRD[0][L] + 1,
                                                                                                   INF)));
    vector<vector<vector<int> > > y(N, vector<vector<int> >(N, vector<int>(biggerRD[0][L] + 1, -1)));
    for (unsigned int s = 0; s < N; s++) { // tamanho da sequencia = s+1

        for (unsigned int f = 0, l = f + s; l <= L; f++, l++) {
            // f: primeiro cliente da sequencla / l: ultimo cliente da sequencia

            // calculamos o tempo para sair do deposito visitar todos os clientes da sequencia, e voltar para o deposito
            unsigned int allClients = sumT[f][l];

            int biggerT = biggerRD[f][l];
            int tFinal = maxT[f][l];
            if (s + 1 == N) {
                tFinal = (int) RD[sequence[f]];
            }
            for (int t = minT[f][l]; t <= tFinal; t++) {
                x[f][l][t] = max(0, biggerT - t); // tempo de espera para iniciar rota com todos clientes
                x[f][l][t] += allClients; // tempo realizar 1 rota visitando todos os clientes

                unsigned int biggerRDlf = iBiggerRD[f][l]; // cliente que tem o maior release date da sequencia
                for (int i = (int) f; i < biggerRDlf; i++) { // testa visitar o deposito depois do cliente i
                    int tFirst = x[f][i][t]; // tempo para realizar a primeira rota

                    unsigned int wSecond = max(0, ((int) RD[sequence[i+1]]) - (t + tFirst));
                    unsigned int startSecond = t + tFirst + wSecond;
                    unsigned int tSecond = x[i + 1][l][min(startSecond,
                                                           maxT[i+1][l])]; // tempo para realizar a segunda rota

                    unsigned int time = tFirst + wSecond + tSecond;

                    if (time < x[f][l][t]) {
                        x[f][l][t] = time;
                        y[f][l][t] = i;
                    }
                }
            }

        }
    } // terminado calculo de x e y
    const unsigned int RD0 = RD[sequence[0]];

    set<unsigned int> visits;
    getDepositsVisits(visits, W, RD, sequence, x, y, 0, L, RD0, *max_element(RD.begin(), RD.end()));

    cout << "PD: ";
    for (auto v: visits) {
        cout << v << "  ";
    }

    cout << endl;
    cout << "Result: " << (RD0 + x[0][L][RD0]) << endl;

    return visits;
}

const vector<unsigned int> &Solution::getSequence() const {
    return sequence;
}
