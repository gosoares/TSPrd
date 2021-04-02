#include "Functions.h"
#include <cstdlib>
#include <chrono>
#include <random>
#include <algorithm>
#include <queue>

#pragma ide diagnostic ignored "cert-msc50-cpp"

double nCloseMean(vector<vector<double> > &d, int n_close, int i);

Sequence *Functions::orderCrossover(const vector<unsigned int> &parent1, const vector<unsigned int> &parent2) {
    int N = parent1.size();

    // escolhe aleatoriamente a subsequencia do primeiro pai que ira ao filho
    int a, b;
    do {
        a = rand() % N;
        b = rand() % N;
        if (a > b) {
            swap(a, b);
        }
    } while ((a == 0 && b == N - 1) || (b - a) <= 1);

    // copia a subsequencia do primeiro pai, ao filho
    vector<bool> has(N, false);
    auto *child = new vector<unsigned int>(N);
    for (int i = a; i <= b; i++) {
        child->at(i) = parent1[i];
        has[child->at(i)] = true;
    }

    // copia o restante dos elementos na ordem que esta no segundo pai
    int x = 0;
    for (int i = b + 1; i < N; i++) {
        while (has[parent2[x]]) // pula os elementos que já foram copiados do primeiro pai
            x++;

        child->at(i) = parent2[x];
        x++;
    }
    for (int i = 0; i < a; i++) {
        while (has[parent2[x]]) // pula os elementos que já foram copiados do primeiro pai
            x++;

        child->at(i) = parent2[x];
        x++;
    }

    return child;
}

double Functions::routesDistance(Solution *r1, Solution *r2) {
    const unsigned int N = r1->V + 1;
    vector<unsigned int> s1Arcs(N), s2Arcs(N); // cada posicao do array representa quem vem depois dele na rota
    set<unsigned int> s1Deposits, s2Deposits; // x: (0, x) existe em rotas

    for (const vector<unsigned int> &route: r1->routes) { // calcula os arcos em s1
        s1Deposits.insert(route[1]);
        for (int i = 2; i < route.size() - 1; i++) {
            s1Arcs[route[i - 1]] = route[i];
        }
    }
    for (const vector<unsigned int> &route: r2->routes) { // calcula os arcos em s2
        s2Deposits.insert(route[1]);
        for (int i = 2; i < route.size() - 1; i++) {
            s2Arcs[route[i - 1]] = route[i];
        }
    }

    int I = 0;
    for (unsigned int x: s1Deposits) { // compara arcos que saem do deposito
        I += s2Deposits.erase(x);
    }
    int U = (int) (s1Deposits.size() + s2Deposits.size());

    for (int i = 1; i < s1Arcs.size(); i++) { // comparam restante das arcos
        int equal = s1Arcs[i] == s2Arcs[i];
        I += equal;
        U += 2 - equal;
    }


    return 1 - ((double) I / U);
}

vector<Sequence *> *Functions::initializePopulation(unsigned int mi, unsigned int lambda) {
    vector<unsigned int> clients(instance.nClients());
    for (int i = 0; i < clients.size(); i++) {
        clients[i] = i + 1;
    }

    auto rand = default_random_engine(chrono::system_clock::now().time_since_epoch().count());

    auto population = new vector<Sequence *>(mi + lambda);
    population->resize(2 * mi); // diminui o tamanho mas mantem o espaco alocado para a quantidade maxima da populacao, para evitar realocacao
    for (int i = 0; i < 2 * mi; i++) {
        auto sequence = new Sequence(clients);
        shuffle(sequence->begin(), sequence->end(), rand);
        population->at(i) = sequence;
    }

    return population;
}

void Functions::getBiasedFitness(vector<double> &biasedFitness, vector<Solution *> *solutions, double nbElite, int nClose) {
    int N = solutions->size();

    vector<vector<double> > d(N, vector<double>(N)); // guarda a distancia entre cada par de cromossomo
    for(int i = 0; i < N; i++) {
        for(int j = i+1; j < N; j++) {
            d[i][j] = Functions::routesDistance(solutions->at(i), solutions->at(j));
            d[j][i] = d[i][j];
        }
    }

    vector<double> nMean(N);
    for (int i = 0; i < N; i++) {
        nMean[i] = nCloseMean(d, nClose, i);
    }

    vector<unsigned int> sortedIndex(N);
    iota(sortedIndex.begin(), sortedIndex.end(), 0);
    sort(sortedIndex.begin(), sortedIndex.end(), [&nMean](int i,int j){
        return nMean[i] < nMean[j];
    });

    vector<unsigned int> rankDiversity(N); // rank em relacao a contribuicao pra diversidade.
    // quanto maior o rank, maior a contribuicao para a diversidade da populacao
    for (int i = 0; i < rankDiversity.size(); i++) {
        rankDiversity[sortedIndex[i]] = i + 1;
    }

    // calcula o rank em relacao ao valor na funcao objetivo dessa solucao
    // quanto menor o valor, melhor a solucao
    sort(sortedIndex.begin(), sortedIndex.end(), [&solutions](int i, int j) {
        return solutions->at(i)->time > solutions->at(i)->time;
    });
    vector<unsigned int> rankFitness(N);
    for (int i = 0; i < rankFitness.size(); i++) {
        rankFitness[sortedIndex[i]] = i + 1;
    }

    // com os ranks, eh calculado o biased fitness
    biasedFitness.resize(solutions->size());
    for (int i = 0; i < biasedFitness.size(); i++) {
        biasedFitness[i] = rankFitness[i] + (1 - ((double) nbElite / solutions->size())) * rankDiversity[i];
    }
}

void Functions::survivalSelection(vector<Solution *> *solutions, int mi, double nbElite, int nClose) {
    vector<double> biasedFitness(solutions->size());
    getBiasedFitness(biasedFitness, solutions, nbElite, nClose);
    for(int i = 0; i < solutions->size(); i++) {
        solutions->at(i)->id = i;
    }

    // ordenamos as solucoes pelo biased fitness e preservamos apenas as mi solucoes com melhores biased fitness
    sort(solutions->begin(), solutions->end() , [&biasedFitness](Solution* s1, Solution* s2) {
        return biasedFitness[s1->id] > biasedFitness[s2->id];
    });
    solutions->resize(mi);
}

void Functions::diversify(vector<Solution *> *solutions, int mi, double nbElite, int nClose) {
    survivalSelection(solutions, mi/3, nbElite, nClose);

    vector<Sequence *> *population = initializePopulation(mi);

    for(auto *sequence: *population) {
        solutions->push_back(new Solution(instance, *sequence));
        delete sequence;
    }
    delete population;
}

double nCloseMean(vector<vector<double> > &d, int n_close, int i) {
    vector<double> &di = d[i];
    // criamos uma fila de prioridade para armazenar as n_close menores distancias
    priority_queue<double> pq;
    // insere as distancias dos primeiros n_close clientes
    int j;
    for (j = 0; pq.size() < n_close; j++) {
        if(i == j)
            continue;
        pq.push(di[j]);
    }

    // para cada distancia restante verifica se eh menor que alguma anterior
    for(; j < d.size(); j++) {
        if (i == j)
            continue;

        unsigned int dij = di[j];
        if(dij < pq.top()) {
            pq.pop();
            pq.push(dij);
        }
    }

    // calcula a media das menores distancias;
    double sum = 0;
    for (j = 0; j < n_close; j++) {
        sum += pq.top();
        pq.pop();
    }

    return sum / n_close;
}
