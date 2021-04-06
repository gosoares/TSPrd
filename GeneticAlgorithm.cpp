#include <chrono>
#include <random>
#include "GeneticAlgorithm.h"
#include <queue>

void freePopulation(vector<Sequence *> *population) {
    for (auto p: *population) {
        delete p;
    }
    population->clear();
}

GeneticAlgorithm::GeneticAlgorithm(
        const string &instanceFile, unsigned int mi, unsigned int lambda, unsigned int nClose, double nbElite,
        unsigned int itNi, unsigned int itDiv
) : instance(instanceFile), mi(mi), lambda(lambda), nClose(nClose), nbElite(nbElite), itNi(itNi), itDiv(itDiv),
    ns(instance) {
    srand(time(nullptr));

    vector<Sequence *> *population = initializePopulation();
    // represents the population for the genetic algorithm
    // the population is simple the big tours (tours sequence) ignoring visits to the depot
    vector<Solution *> *solutions = Solution::solutionsFromSequences(instance, population);
    // represents the solution itself, with a set of routes
    // during the genetic algorithm execution, we frequently transforms the population in solutions
    // applying the split algorithm which find the optimal depot visits for each sequence;
    // and we transform the solutions in a population, removing the depot visits from the solutions
    // and constructing a vector with only the clients visit sequence
    auto *bestSolution = new Solution(vector<vector<unsigned int> >(), 999999);

    int iterations_not_improved = 0;

    while (iterations_not_improved < this->itNi) {
        vector<double> biasedFitness = getBiasedFitness(solutions);

        while (solutions->size() < mi + lambda) {
            // SELECAO DOS PARENTES PARA CROSSOVER
            vector<unsigned int> p = selectParents(biasedFitness);

            Sequence *child = GeneticAlgorithm::orderCrossover(*population->at(p[0]), *population->at(p[1]));
            auto *sol = new Solution(instance, *child);
            delete child;

            // EDUCACAO
            ns.educate(sol);
            solutions->push_back(sol);

            if (sol->time < bestSolution->time) {
                delete bestSolution;
                bestSolution = sol->copy();
                // cout << "Best Solution Found: " << bestSolution->time << endl;

                iterations_not_improved = 0;
            } else {
                iterations_not_improved++;
                if (iterations_not_improved == this->itDiv) { // DIVERSIFICACAO
                    diversify(solutions);
                }
            }
        }

        survivalSelection(solutions);

        //recalculate population
        freePopulation(population);
        for (Solution *s: *solutions) {
            population->push_back(s->toSequence());
        }
    }

    this->solution = bestSolution;
}

vector<Sequence *> *GeneticAlgorithm::initializePopulation() {
    // initial population will be of size 2*mi generated randomly
    vector<unsigned int> clients(instance.nClients()); // represents the sequence of clients visiting (big tour)
    // will be shuffled for each element of the population
    iota(clients.begin(), clients.end(), 1);

    auto rand = default_random_engine(chrono::system_clock::now().time_since_epoch().count());

    auto pop = new vector<Sequence *>(mi + lambda);
    pop->resize(2 *
                mi); // diminui o tamanho mas mantem o espaco alocado para a quantidade maxima da populacao, para evitar realocacao
    for (int i = 0; i < 2 * mi; i++) {
        auto sequence = new Sequence(clients);
        shuffle(sequence->begin(), sequence->end(), rand);
        pop->at(i) = sequence;
    }

    return pop;
}

double solutionsDistances(Solution *s1, Solution *s2) {
    const unsigned int N = s1->N + 1;
    vector<unsigned int> s1Arcs(N), s2Arcs(N); // cada posicao do array representa quem vem depois dele na rota
    set<unsigned int> s1Depots, s2Depots; // x: (0, x) existe em rotas

    for (const vector<unsigned int> &route: s1->routes) { // calcula os arcos em s1
        s1Depots.insert(route[1]);
        for (int i = 2; i < route.size() - 1; i++) {
            s1Arcs[route[i - 1]] = route[i];
        }
    }
    for (const vector<unsigned int> &route: s2->routes) { // calcula os arcos em s2
        s2Depots.insert(route[1]);
        for (int i = 2; i < route.size() - 1; i++) {
            s2Arcs[route[i - 1]] = route[i];
        }
    }

    int I = 0;
    for (unsigned int x: s1Depots) { // compara arcos que saem do deposito
        I += s2Depots.erase(x);
    }
    int U = (int) (s1Depots.size() + s2Depots.size());

    for (int i = 1; i < s1Arcs.size(); i++) { // comparam restante das arcos
        int equal = s1Arcs[i] == s2Arcs[i];
        I += equal;
        U += 2 - equal;
    }


    return 1 - ((double) I / U);
}

double nCloseMean(const vector<vector<double> > &d, unsigned int n_close, unsigned int i) {
    const vector<double> &di = d[i];
    // criamos uma fila de prioridade para armazenar as n_close menores distancias
    priority_queue<double> pq;
    // insere as distancias dos primeiros n_close clientes
    int j;
    for (j = 0; pq.size() < n_close; j++) {
        if (i == j)
            continue;
        pq.push(di[j]);
    }

    // para cada distancia restante verifica se eh menor que alguma anterior
    for (; j < d.size(); j++) {
        if (i == j)
            continue;

        unsigned int dij = di[j];
        if (dij < pq.top()) {
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

vector<double> GeneticAlgorithm::getBiasedFitness(vector<Solution *> *solutions) const {
    int N = solutions->size();
    vector<double> biasedFitness(N);

    vector<vector<double> > d(N, vector<double>(N)); // guarda a distancia entre cada par de cromossomo
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            d[i][j] = solutionsDistances(solutions->at(i), solutions->at(j));
            d[j][i] = d[i][j];
        }
    }

    vector<double> nMean(N);
    for (int i = 0; i < N; i++) {
        nMean[i] = nCloseMean(d, nClose, i);
    }

    vector<unsigned int> sortedIndex(N);
    iota(sortedIndex.begin(), sortedIndex.end(), 0);
    sort(sortedIndex.begin(), sortedIndex.end(), [&nMean](int i, int j) {
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

    return biasedFitness;
}

vector<unsigned int> GeneticAlgorithm::selectParents(vector<double> &biasedFitness) const {
    // seleciona o primeiro pai
    vector<unsigned int> p(2);
    unsigned int p1a = rand() % mi, p1b = rand() % mi;
    p[0] = p1a;
    if (biasedFitness[p1b] > biasedFitness[p1a])
        p[0] = p1b;


    // seleciona segundo pai, diferente do primeiro
    do {
        int p2a = rand() % mi, p2b = rand() % mi;
        p[1] = p2a;
        if (biasedFitness[p2b] > biasedFitness[p2a])
            p[1] = p2b;
    } while (p[0] == p[1]);

    return p;
}

Sequence *GeneticAlgorithm::orderCrossover(const Sequence &parent1, const Sequence &parent2) {
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

void GeneticAlgorithm::survivalSelection(vector<Solution *> *solutions, unsigned int Mi) {
    vector<double> biasedFitness = getBiasedFitness(solutions);
    for (int i = 0; i < solutions->size(); i++) {
        solutions->at(i)->id = i;
    }

    // sort the solutions based on the biased fitness and keep only the best 'mi' solutions
    sort(solutions->begin(), solutions->end(), [&biasedFitness](Solution *s1, Solution *s2) {
        return biasedFitness[s1->id] > biasedFitness[s2->id];
    });
    solutions->resize(Mi);
}

void GeneticAlgorithm::diversify(vector<Solution *> *solutions) {
    survivalSelection(solutions, mi / 3); // keeps the mi/3 best solutions we have so far

    // generate more solutions with the same procedure that generated the initial population
    // and appends to the current solutions
    vector<Sequence *> *population = initializePopulation();

    for (auto *sequence: *population) {
        solutions->push_back(new Solution(instance, *sequence));
        delete sequence;
    }
    delete population;
}