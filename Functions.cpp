#include "Functions.h"
#include <cstdlib>
#include <chrono>
#include <random>
#include <algorithm>

#pragma ide diagnostic ignored "cert-msc50-cpp"
void Functions::orderCrossover(const vector<unsigned int> &parent1, const vector<unsigned int> &parent2, vector<unsigned int> &child) {
    int N = parent1.size();

    random_device dev;
    std::default_random_engine generator(dev());
    std::uniform_int_distribution<int> distribution(1,6);
    int dice_roll = distribution(generator);  // generates number in the range 1..6


    // escolhe aleatoriamente a subsequencia do primeiro pai que ira ao filho
    int a, b;
    do {
        a = rand() % N;
        b = rand() % N;
        if (a > b) {
            swap(a, b);
        }
    } while(a == 0 && b == N-1);


    // copia a subsequencia do primeiro pai, ao filho
    vector<bool> has(N, false);
    child.resize(N);
    for (int i = a; i <= b; i++) {
        child[i] = parent1[i];
        has[child[i]] = true;
    }

    // copia o restante dos elementos na ordem que esta no segundo pai
    int x = 0;
    for (int i = b+1; i < N; i++) {
        while(has[parent2[x]]) // pula os elementos que já foram copiados do primeiro pai
            x++;

        child[i] = parent2[x];
        x++;
    }
    for (int i = 0; i < a; i++) {
        while(has[parent2[x]]) // pula os elementos que já foram copiados do primeiro pai
            x++;

        child[i] = parent2[x];
        x++;
    }
}

double Functions::solutionsDistance(const Solution &s1, const Solution &s2) {
    const unsigned int N = s1.getInstance().nVertex();
    vector<unsigned int> s1Arcs(N), s2Arcs(N); // cada posicao do array representa quem vem depois dele na rota
    set<unsigned int> s1Deposits, s2Deposits; // x: (0, x) existe em rotas

    for(const vector<unsigned int> &route: s1.getRoutes()) { // calcula os arcos em s1
        s1Deposits.insert(route[1]);
        for (int i = 2; i < route.size() - 1; i++) {
            s1Arcs[route[i-1]] = route[i];
        }
    }
    for(const vector<unsigned int> &route: s2.getRoutes()) { // calcula os arcos em s2
        s2Deposits.insert(route[1]);
        for (int i = 2; i < route.size() - 1; i++) {
            s2Arcs[route[i-1]] = route[i];
        }
    }

    int I = 0;
    for(unsigned int x: s1Deposits) { // compara arcos que saem do deposito
        I += s2Deposits.erase(x);
    }
    int U = (int) (s1Deposits.size() + s2Deposits.size());

    for(int i = 1; i < s1Arcs.size(); i++) { // comparam restante das arcos
        int equal = s1Arcs[i] == s2Arcs[i];
        I += equal;
        U += 2 - equal;
    }


    return 1 - ((double) I/U);
}

/*
 * population: vector que representa a populacao
 * M: numero de cromossomos na populacao inicial
 */
void Functions::initializePopulation(const Instance &instance, vector<vector<Solution *>> &population, unsigned int M) {
    vector<unsigned int> clients(instance.nClients());
    for (int i = 0; i < clients.size(); i++) {
        clients[i] = i+1;
    }

    auto rand = default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    population.resize(M);
    for (int i = 0; i < M; i++) {
        population[i] = clients;
        shuffle(population[i].begin(), population[i].end(), rand);
    }
}

void Functions::survivalSelection(vector<vector<Solution>> &population, int pop_size) {

}


