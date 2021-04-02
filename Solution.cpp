#include <limits>
#include <algorithm>
#include "Solution.h"
#include <iostream>

using namespace std;

#pragma ide diagnostic ignored "modernize-pass-by-value"

unsigned int routesFromSequence(const Instance &instance, const vector<unsigned int> &sequence,
                                vector<vector<unsigned int> > &routes, bool reduce = false);

unsigned int split(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &S);

unsigned int
visitsFromSequence(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &sequence);

unsigned int visitsFromSequenceReducing(set<unsigned int> &visits, const vector<vector<unsigned int> > &W,
                                        const vector<unsigned int> &RD, const vector<unsigned int> &sequence);

Solution::Solution(vector<vector<unsigned int> > routes, unsigned int time, int N): routes(std::move(routes)), time(time), V(N) {
    if(N == -1) {
        this->V = 0;
        for (auto &r: routes) {
            this->V += r.size() - 2;
        }
    }
};

Solution::Solution(const Instance &instance, Sequence &sequence, bool reduce): V(sequence.size()) {
    time = routesFromSequence(instance, sequence, routes, reduce);
}

vector<Solution *> *Solution::solutionsFromSequences(const Instance &instance, vector<Sequence *> *sequences, bool reduce) {
    auto *solutions = new vector<Solution *>(sequences->size());
    for (int i = 0; i < solutions->size(); i++ ){
        solutions->at(i) = new Solution(instance, *(sequences->at(i)), reduce);
    }
    return solutions;
}

Solution *Solution::copy() const{
    vector<vector<unsigned int> > r(this->routes);
    return new Solution(r, this->time);
}

Sequence *Solution::getSequence() const {
    auto *s = new Sequence(this->V);
    int i = 0;
    for(const vector<unsigned int> &route: routes) {
        for(int j = 1; j < route.size() - 1; j++) {
            s->at(i) = route[j];
            i++;
        }
    }
    return s;
}

unsigned int Solution::getRoutesTime(const Instance &instance, const vector<vector<unsigned int> >& routes) {
    unsigned int time = 0;
    for (const vector<unsigned int> &route : routes) {
        // get max release date of elements in this route
        unsigned int maxRD = 0;
        for(int i = 1; i < route.size() - 1; i++) {
            unsigned int rdi = instance.releaseTimeOf(route[i]);
            if(rdi > maxRD) {
                maxRD = rdi;
            }
        }

        if (time < maxRD) {
            time = maxRD; // time = horario de saida da rota
        }

        // acrescenta tempo de realizar a rota
        for(int i = 1; i < route.size(); i++) {
            time += instance.time(route[i-1], route[i]);
        }
    }

    return time;
}

/*
 * dada uma sequencia, calcula o melhor conjunto de rotas que visitem os clientes nessa sequencia
 *
 * retorna o tempo de realizar a rota, e altera o "routes" com a rota
 */

void printset(set<unsigned int> s) {
    for (auto a: s) {
        cout << a << " ";
    }
}
unsigned int routesFromSequence(const Instance &instance, const vector<unsigned int> &sequence,
                                vector<vector<unsigned int> > &routes, bool reduce) {
    set<unsigned int> depositVisits;
    unsigned int time;
    if (reduce) {
        time = visitsFromSequenceReducing(depositVisits, instance.getW(), instance.getRD(), sequence);
    } else {
        time = visitsFromSequence(depositVisits, instance.getW(), instance.getRD(), sequence);
//        set<unsigned int> dv2;
//        unsigned int time2;
//        time2 = split(dv2, instance.getW(), instance.getRD(), sequence);
//        printset(depositVisits);
//        cout << time << endl;
//        printset(dv2);
//        cout << time2 << endl;
//        cout << (time == time2 ? "Igual" : "Diferente") << endl << endl << endl;
    }


    routes.push_back(vector<unsigned int>({0}));
    for (unsigned int i : sequence) {
        routes.back().push_back(i);

        const bool is_in = depositVisits.find(i) != depositVisits.end();
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

    return time;
}


void
getDepositsVisits(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                  const vector<unsigned int> &sequence, const vector<vector<vector<unsigned int> > > &x,
                  const vector<vector<vector<int> > > &y, unsigned int i, unsigned int j, unsigned int t,
                  unsigned int biggerRD) {
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

unsigned int split(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &S) {
    const unsigned int V = RD.size(), // numero total de vértices, incluindo o deposito
    N = V - 1; // numero total de clientes

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

    for(int i = 0; i < N; i++) {
        for(int j = i + 1; j <= N; j++) {
            unsigned int deltaJ = max(biggerRD[i][j-1], delta[i]) + sumT[i][j-1];
            if(deltaJ < delta[j]) {
                delta[j] = deltaJ;
                bestIn[j] = i;
            }
        }
    }

    unsigned int x = bestIn[N];
    while(x > 0) {
        visits.insert(S[x-1]);
        x = bestIn[x];
    }

    return delta.back();
}

// dada uma sequencia, calcula onde colocar os depositos para se obter o menor tempo total
unsigned int
visitsFromSequence(set<unsigned int> &visits, const vector<vector<unsigned int> > &W, const vector<unsigned int> &RD,
                   const vector<unsigned int> &sequence) {
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
        if (i > 0) {
            //int ibiggerRDbeforei = iBiggerRD[0][i - 1];
            unsigned int biggerRDbeforei = biggerRD[0][i - 1];
            mint = biggerRDbeforei; // + sumT[ibiggerRDbeforei][i-1];
        }
        mint = max(mint, RD[sequence[i]]);

        for (int j = i; j < sequence.size(); j++) {
            minT[i][j] = mint;

            //unsigned int maxt = biggerRDbeforei + sumT[0][i-1];
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

                unsigned int biggerRDfl = iBiggerRD[f][l]; // cliente que tem o maior release date da sequencia
                for (int i = (int) f; i < biggerRDfl; i++) { // testa visitar o deposito depois do cliente i
                    int tFirst = x[f][i][t]; // tempo para realizar a primeira rota

                    unsigned int wSecond = max(0, ((int) RD[sequence[i + 1]]) - (t + tFirst));
                    unsigned int startSecond = t + tFirst + wSecond;
                    unsigned int tSecond = x[i + 1][l][min(startSecond,
                                                           maxT[i + 1][l])]; // tempo para realizar a segunda rota

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

    getDepositsVisits(visits, W, RD, sequence, x, y, 0, L, RD0, *max_element(RD.begin(), RD.end()));

//    cout << "PD: ";
//    for (auto v: visits) {
//        cout << v << "  ";
//    }
//
//    cout << endl;
//    cout << "Result: " << (RD0 + x[0][L][RD0]) << endl;

    return RD0 + x[0][L][RD0];
}

unsigned int visitsFromSequenceReducing(set<unsigned int> &visits, const vector<vector<unsigned int> > &W,
                                        const vector<unsigned int> &RD, const vector<unsigned int> &sequence) {
/*
     * Dado uma sequência fixa, se um cliente tem um release date maior que o proximo
     * cliente da sequencia, então os dois clientes serão atendidos na mesma rota.
     *
     * Com essa ideia podemos agrupa-los em apenas 1 vértice representando os clientes naquela sequencia.
     *
     * */
    vector<vector<unsigned int> > mapping;
    mapping.push_back({0});
    mapping.push_back({sequence[0]});
    unsigned int bigger = RD[sequence[0]];
    vector<unsigned int> newReleaseTime({RD[0], RD[sequence[0]]});

    for (int i = 1; i < sequence.size(); i++) {
        unsigned int v = sequence[i];
        unsigned int rdi = RD[v];

        if (rdi > bigger) {
            mapping.push_back({v});
            bigger = rdi;
            newReleaseTime.push_back(rdi);
        } else {
            mapping.back().push_back(v);
        }
    }
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
            inner += W[mapping[i][j - 1]][mapping[i][j]];
        }

        newGraph[0][i] = W[0][mapping[i][0]];
        newGraph[i][0] = inner + W[mapping[i].back()][0];

        if (i + 1 < mapping.size()) {
            newGraph[i][i + 1] = inner + W[mapping[i].back()][mapping[i + 1][0]];
        }

    }

    vector<unsigned int> newSequence(newGraph.size() - 1);
    for (int i = 0; i < newSequence.size(); i++)
        newSequence[i] = i + 1;

    set<unsigned int> mappedVisits;
    unsigned int time = visitsFromSequence(mappedVisits, newGraph, newReleaseTime, newSequence);

    for (unsigned int v: mappedVisits) {
        visits.insert(mapping[v].back());
    }

//    cout << "PD2: ";
//    for (auto v: visits) {
//        cout << v << "  ";
//    }
//    cout << endl;

    return time;
}