#include "GeneticAlgorithm.h"

#include <chrono>
#include <queue>

#include "Rng.h"

void freePopulation(vector<Sequence*>* population) {
    for (auto p : *population) {
        delete p;
    }
    population->clear();
}

GeneticAlgorithm::GeneticAlgorithm(const Instance& instance, unsigned int mi, unsigned int lambda, unsigned int nClose,
                                   unsigned int nbElite, unsigned int itNi, unsigned int itDiv, unsigned int timeLimit)
    : instance(instance), mi(mi), lambda(lambda), nbElite(nbElite), nClose(nClose), itNi(itNi), itDiv(itDiv),
      timeLimit(timeLimit), ns(instance), endTime(0), bestSolutionFoundTime(0),
      populationRandom(Rng::getIntGenerator(0, (int)mi - 1)) {
    milliseconds maxTime(this->timeLimit * 1000);
    timer.start();

    vector<Sequence*>* population = initializePopulation();
    // represents the population for the genetic algorithm
    // the population is simple the big tours (tours sequence) ignoring visits to the depot
    vector<Solution*>* solutions = Solution::solutionsFromSequences(instance, population);
    // represents the solution itself, with a set of routes
    // during the genetic algorithm execution, we frequently transform the population in solutions
    // applying the split algorithm which find the optimal depot visits for each sequence;
    // and we transform the solutions in a population, removing the depot visits from the solutions
    // and constructing a vector with only the clients visit sequence

    // get the best solution in the initial population
    bestSolution = nullptr;
    for (auto s : *solutions) {
        if (bestSolution == nullptr || s->time < bestSolution->time) {
            bestSolution = s;
        }
    }
    bestSolution = bestSolution->copy();
    searchProgress.emplace_back(timer.elapsedTime().count(), bestSolution->time);

    unsigned int iterations_not_improved = 0;

    while (iterations_not_improved < this->itNi && timer.elapsedTime() < maxTime) {
        vector<double> biasedFitness = getBiasedFitness(solutions);

        while (solutions->size() < mi + lambda) {
            // SELECAO DOS PARENTES PARA CROSSOVER
            vector<unsigned int> p = selectParents(biasedFitness);

            Sequence* child = GeneticAlgorithm::orderCrossover(*population->at(p[0]), *population->at(p[1]));
            auto* sol = new Solution(instance, *child);
            delete child;

            // EDUCACAO
            ns.educate(sol);
            solutions->push_back(sol);

            if (sol->time < bestSolution->time) {
                bestSolutionFoundTime = timer.elapsedTime();
                delete bestSolution;
                bestSolution = sol->copy();
                // cout << "Best Solution Found: " << bestSolution->time << endl;
                searchProgress.emplace_back(bestSolutionFoundTime.count(), bestSolution->time);

                iterations_not_improved = 0;
            } else {
                iterations_not_improved++;
                if (iterations_not_improved % this->itDiv == 0) {
                    diversify(solutions);
                } else if (iterations_not_improved == this->itNi) {
                    break;
                }
            }
            if (timer.elapsedTime() > maxTime) break;  // time limit
        }

        survivalSelection(solutions);

        // recalculate population
        freePopulation(population);
        for (Solution* s : *solutions) {
            population->push_back(s->toSequence());
        }
    }

    endTime = timer.elapsedTime();
}

vector<Sequence*>* GeneticAlgorithm::initializePopulation() {
    // initial population will be of size 2*mu generated randomly
    vector<unsigned int> clients(instance.nClients());  // represents the sequence of clients visiting (big tour)
    // will be shuffled for each element of the population
    iota(clients.begin(), clients.end(), 1);

    auto pop = new vector<Sequence*>(mi + lambda);
    pop->resize(2 * mi);
    // reduces the size of the vector, but keeps enough space to store the max population without needing reallocation

    for (unsigned int i = 0; i < 2 * mi; i++) {
        auto sequence = new Sequence(clients);
        shuffle(sequence->begin(), sequence->end(), Rng::getGenerator());
        pop->at(i) = sequence;
    }

    return pop;
}

/**
 *
 * @param s1 first solution to be compared
 * @param s2 second solution to be compared
 * @return distance between the solutions
 *
 * the distance is calculated based in how many arcs the routes from the solutions have in common
 */
double GeneticAlgorithm::solutionsDistances(Solution* s1, Solution* s2, bool symmetric) {
    unsigned int V = s1->N + 1, a, b;
    unsigned int I = 0;  // number of arcs that exists in both solutions
    unsigned int U = 0;  // number of arcs in Arcs(s1) U Arcs(s2)

    vector<set<unsigned int>> adjList1(V);
    for (auto& route : s1->routes) {
        for (unsigned int c = 1; c < route->size(); c++) {
            U++;  // count arcs in s1
            a = route->at(c - 1);
            b = route->at(c);
            adjList1[a].insert(b);
            if (symmetric) adjList1[b].insert(a);
        }
    }

    for (auto& route : s2->routes) {
        for (unsigned int c = 1; c < route->size(); c++) {
            a = route->at(c - 1);
            b = route->at(c);
            if (adjList1[a].find(b) != adjList1[a].end()) {
                I++;
            } else {
                U++;
            }
        }
    }

    return 1 - ((double)I / U);
}

/**
 *
 * @param d matrix of the distances of the solutions
 * @param n_close how many closest solutions should the algorithm consider to calculate the mean
 * @param i which solution to calculate the mean
 * @return the mean of the distances of the solution i to the 'nClose' closest solutions
 */
double nCloseMean(const vector<vector<double>>& d, unsigned int n_close, unsigned int i) {
    const vector<double>& di = d[i];
    // criamos uma fila de prioridade para armazenar as n_close menores distancias
    priority_queue<double> pq;
    // insere as distancias dos primeiros n_close clientes
    unsigned int j;
    for (j = 0; pq.size() < n_close; j++) {
        if (i == j) continue;
        pq.push(di[j]);
    }

    // para cada distancia restante verifica se eh menor que alguma anterior
    for (; j < d.size(); j++) {
        if (i == j) continue;

        double dij = di[j];
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

vector<double> GeneticAlgorithm::getBiasedFitness(vector<Solution*>* solutions) const {
    unsigned int N = solutions->size();  // nbIndiv
    vector<double> biasedFitness(N);

    vector<vector<double>> d(N, vector<double>(N));  // guarda a distancia entre cada par de cromossomo
    for (unsigned int i = 0; i < N; i++) {
        for (unsigned int j = i + 1; j < N; j++) {
            d[i][j] = solutionsDistances(solutions->at(i), solutions->at(j), instance.isSymmetric());
            d[j][i] = d[i][j];
        }
    }

    vector<double> nMean(N);
    for (unsigned int i = 0; i < N; i++) {
        nMean[i] = nCloseMean(d, nClose, i);
    }

    vector<unsigned int> sortedIndex(N);
    iota(sortedIndex.begin(), sortedIndex.end(), 0);
    sort(sortedIndex.begin(), sortedIndex.end(), [&nMean](int i, int j) { return nMean[i] > nMean[j]; });

    vector<unsigned int> rankDiversity(N);  // rank of the solution with respect to the diversity contribution
    // best solutions (higher nMean distance) have the smaller ranks
    for (unsigned int i = 0; i < rankDiversity.size(); i++) {
        rankDiversity[sortedIndex[i]] = i + 1;
    }

    // rank with respect to the time of the solution
    // best solutions (smaller times) have the smaller ranks
    sort(sortedIndex.begin(), sortedIndex.end(),
         [&solutions](int i, int j) { return solutions->at(i)->time < solutions->at(j)->time; });
    vector<unsigned int> rankFitness(N);
    for (unsigned int i = 0; i < rankFitness.size(); i++) {
        rankFitness[sortedIndex[i]] = i + 1;
    }

    // calculate the biased fitness with the equation 4 in vidal article's
    // best solutions have smaller biased fitness
    biasedFitness.resize(solutions->size());
    for (unsigned int i = 0; i < biasedFitness.size(); i++) {
        biasedFitness[i] = rankFitness[i] + (1 - ((double)nbElite / N)) * rankDiversity[i];
    }

    return biasedFitness;
}

vector<unsigned int> GeneticAlgorithm::selectParents(vector<double>& biasedFitness) {
    // select first parent
    vector<unsigned int> p(2);
    int p1a = populationRandom(), p1b = populationRandom();
    p[0] = p1a;
    if (biasedFitness[p1b] < biasedFitness[p1a]) p[0] = p1b;

    // select second parent, different from first
    do {
        int p2a = populationRandom(), p2b = populationRandom();
        p[1] = p2a;
        if (biasedFitness[p2b] < biasedFitness[p2a]) p[1] = p2b;
    } while (p[0] == p[1]);

    return p;
}

Sequence* GeneticAlgorithm::orderCrossover(const Sequence& parent1, const Sequence& parent2) {
    unsigned int N = parent1.size();
    auto dist = Rng::getIntGenerator(0, (int)N - 1);

    // choose randomly a sub-sequence of the first parent that goes to the offspring
    // a and b represents the start and end index of the sequence, respectively
    unsigned int a, b;
    do {
        a = dist();
        b = dist();
        if (a > b) {
            swap(a, b);
        }
    } while ((a == 0 && b == N - 1) || (b - a) <= 1);

    // copy the chosen subsequence from the first parent to the offspring
    vector<bool> has(N, false);
    auto* offspring = new vector<unsigned int>(N);
    for (unsigned int i = a; i <= b; i++) {
        offspring->at(i) = parent1[i];
        has[offspring->at(i)] = true;
    }

    // copy the remaining elements keeping the relative order that they appear in the second parent
    int x = 0;
    for (unsigned int i = 0; i < a; i++) {
        while (has[parent2[x]])  // pula os elementos que já foram copiados do primeiro pai
            x++;

        offspring->at(i) = parent2[x];
        x++;
    }
    for (unsigned int i = b + 1; i < N; i++) {
        while (has[parent2[x]])  // pula os elementos que já foram copiados do primeiro pai
            x++;

        offspring->at(i) = parent2[x];
        x++;
    }

    return offspring;
}

void GeneticAlgorithm::survivalSelection(vector<Solution*>* solutions, unsigned int Mi) {
    vector<double> biasedFitness = getBiasedFitness(solutions);
    for (unsigned int i = 0; i < solutions->size(); i++) {
        solutions->at(i)->id = i;
    }

    // look for clones and give them a very low biased fitness
    // this way they will be removed from population
    const double INF = instance.nVertex() * 10;
    vector<bool> isClone(solutions->size(), false);
    for (unsigned int i = 0; i < solutions->size(); i++) {
        if (isClone[i]) continue;
        for (unsigned int j = i + 1; j < solutions->size(); j++) {
            if (solutions->at(i)->equals(solutions->at(j))) {
                isClone[j] = true;
                biasedFitness[j] += INF;
            }
        }
    }

    // sort the solutions based on the biased fitness and keep only the best 'mu' solutions
    sort(solutions->begin(), solutions->end(),
         [&biasedFitness](Solution* s1, Solution* s2) { return biasedFitness[s1->id] < biasedFitness[s2->id]; });
    for (unsigned int c = Mi + 1; c < solutions->size(); c++) {
        delete solutions->at(c);
    }
    solutions->resize(Mi);
}

void GeneticAlgorithm::diversify(vector<Solution*>* solutions) {
    survivalSelection(solutions, mi / 3);  // keeps the mi/3 best solutions we have so far

    // generate more solutions with the same procedure that generated the initial population
    // and appends to the current solutions
    vector<Sequence*>* population = initializePopulation();

    for (auto* sequence : *population) {
        auto* s = new Solution(instance, *sequence);
        if (s->time < this->bestSolution->time) {
            delete this->bestSolution;
            this->bestSolution = s->copy();
            searchProgress.emplace_back(timer.elapsedTime().count(), s->time);
        }
        solutions->push_back(s);
        delete sequence;
    }
    delete population;
}
