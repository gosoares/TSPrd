#include "GeneticAlgorithm.h"

void freePopulation(std::vector<Sequence*>* population) {
    for (auto p : *population) {
        delete p;
    }
    population->clear();
}

GeneticAlgorithm::GeneticAlgorithm(Data& data)
    : data(data), ns(data), endTime(0), bestSolutionFoundTime(0), distPopulation(0, (int)data.params.mu - 1) {
    std::chrono::milliseconds maxTime(this->data.params.timeLimit * 1000);

    std::vector<Sequence*>* population = initializePopulation();
    // represents the population for the genetic algorithm
    // the population is simple the big tours (tours sequence) ignoring visits to the depot
    std::vector<Solution*>* solutions = Solution::solutionsFromSequences(data, population);
    // represents the solution itself, with a set of routes
    // during the genetic algorithm execution, we frequently transform the population in solutions
    // applying the split algorithm which find the optimal depot visits for each sequence;
    // and we transform the solutions in a population, removing the depot visits from the solutions
    // and constructing a std::vector with only the clients visit sequence

    // get the best solution in the initial population
    bestSolution = nullptr;
    for (auto s : *solutions) {
        if (bestSolution == nullptr || s->time < bestSolution->time) {
            bestSolution = s;
        }
    }
    bestSolution = bestSolution->copy();
    searchProgress.emplace_back(std::chrono::steady_clock::now(), bestSolution->time);

    int iterations_not_improved = 0;

    while (iterations_not_improved < this->data.params.itNi && data.elapsedTime() < maxTime) {
        std::vector<double> biasedFitness = getBiasedFitness(solutions);

        while (solutions->size() < data.params.mu + data.params.lambda) {
            // SELECAO DOS PARENTES PARA CROSSOVER
            std::vector<int> p = selectParents(biasedFitness);

            Sequence* child = GeneticAlgorithm::orderCrossover(*population->at(p[0]), *population->at(p[1]));
            auto* sol = new Solution(data, *child);
            delete child;

            // EDUCACAO
            ns.educate(sol);
            solutions->push_back(sol);

            if (sol->time < bestSolution->time) {
                bestSolutionFoundTime = data.elapsedTime();
                delete bestSolution;
                bestSolution = sol->copy();
                // cout << "Best Solution Found: " << bestSolution->time << endl;
                searchProgress.emplace_back(bestSolutionFoundTime, bestSolution->time);

                iterations_not_improved = 0;
            } else {
                iterations_not_improved++;
                if (iterations_not_improved % data.params.itDiv == 0) {
                    diversify(solutions);
                } else if (iterations_not_improved == data.params.itNi) {
                    break;
                }
            }
            if (data.elapsedTime() > maxTime) break;  // time limit
        }

        survivalSelection(solutions);

        // recalculate population
        freePopulation(population);
        for (Solution* s : *solutions) {
            population->push_back(s->toSequence());
        }
    }

    endTime = data.elapsedTime();
}

std::vector<Sequence*>* GeneticAlgorithm::initializePopulation() {
    // initial population will be of size 2*mu generated randomly
    std::vector<int> clients(data.N);  // represents the sequence of clients visiting (big tour)
    // will be shuffled for each element of the population
    iota(clients.begin(), clients.end(), 1);

    auto pop = new std::vector<Sequence*>(data.params.mu + data.params.lambda);
    pop->resize(2 * data.params.mu);
    // reduces the size of the std::vector, but keeps enough space to store the max population without needing
    // reallocation

    for (int i = 0; i < 2 * data.params.mu; i++) {
        auto sequence = new Sequence(clients);
        shuffle(sequence->begin(), sequence->end(), data.generator);
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
    int V = s1->N + 1, a, b;
    int I = 0;  // number of arcs that exists in both solutions
    int U = 0;  // number of arcs in Arcs(s1) U Arcs(s2)

    std::vector<std::set<int>> adjList1(V);
    for (auto& route : s1->routes) {
        for (int c = 1; c < route->size(); c++) {
            U++;  // count arcs in s1
            a = route->at(c - 1);
            b = route->at(c);
            adjList1[a].insert(b);
            if (symmetric) adjList1[b].insert(a);
        }
    }

    for (auto& route : s2->routes) {
        for (int c = 1; c < route->size(); c++) {
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
double nCloseMean(const std::vector<std::vector<double>>& d, int n_close, int i) {
    const std::vector<double>& di = d[i];
    // criamos uma fila de prioridade para armazenar as n_close menores distancias
    std::priority_queue<double> pq;
    // insere as distancias dos primeiros n_close clientes
    int j;
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

std::vector<double> GeneticAlgorithm::getBiasedFitness(std::vector<Solution*>* solutions) const {
    int N = solutions->size();  // nbIndiv
    std::vector<double> biasedFitness(N);

    std::vector<std::vector<double>> d(N, std::vector<double>(N));  // guarda a distancia entre cada par de cromossomo
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            d[i][j] = solutionsDistances(solutions->at(i), solutions->at(j), data.symmetric);
            d[j][i] = d[i][j];
        }
    }

    std::vector<double> nMean(N);
    for (int i = 0; i < N; i++) {
        nMean[i] = nCloseMean(d, data.params.nClose, i);
    }

    std::vector<int> sortedIndex(N);
    iota(sortedIndex.begin(), sortedIndex.end(), 0);
    sort(sortedIndex.begin(), sortedIndex.end(), [&nMean](int i, int j) { return nMean[i] > nMean[j]; });

    std::vector<int> rankDiversity(N);  // rank of the solution with respect to the diversity contribution
    // best solutions (higher nMean distance) have the smaller ranks
    for (int i = 0; i < rankDiversity.size(); i++) {
        rankDiversity[sortedIndex[i]] = i + 1;
    }

    // rank with respect to the time of the solution
    // best solutions (smaller times) have the smaller ranks
    sort(sortedIndex.begin(), sortedIndex.end(),
         [&solutions](int i, int j) { return solutions->at(i)->time < solutions->at(j)->time; });
    std::vector<int> rankFitness(N);
    for (int i = 0; i < rankFitness.size(); i++) {
        rankFitness[sortedIndex[i]] = i + 1;
    }

    // calculate the biased fitness with the equation 4 in vidal article's
    // best solutions have smaller biased fitness
    biasedFitness.resize(solutions->size());
    for (int i = 0; i < biasedFitness.size(); i++) {
        biasedFitness[i] = rankFitness[i] + (1 - ((double)data.params.nbElite / N)) * rankDiversity[i];
    }

    return biasedFitness;
}

std::vector<int> GeneticAlgorithm::selectParents(std::vector<double>& biasedFitness) {
    // select first parent
    std::vector<int> p(2);
    int p1a = distPopulation(data.generator), p1b = distPopulation(data.generator);
    p[0] = p1a;
    if (biasedFitness[p1b] < biasedFitness[p1a]) p[0] = p1b;

    // select second parent, different from first
    do {
        int p2a = distPopulation(data.generator), p2b = distPopulation(data.generator);
        p[1] = p2a;
        if (biasedFitness[p2b] < biasedFitness[p2a]) p[1] = p2b;
    } while (p[0] == p[1]);

    return p;
}

Sequence* GeneticAlgorithm::orderCrossover(const Sequence& parent1, const Sequence& parent2) {
    int N = parent1.size();
    std::uniform_int_distribution<int> dist(0, (int)N - 1);

    // choose randomly a sub-sequence of the first parent that goes to the offspring
    // a and b represents the start and end index of the sequence, respectively
    int a, b;
    do {
        a = dist(data.generator);
        b = dist(data.generator);
        if (a > b) {
            std::swap(a, b);
        }
    } while ((a == 0 && b == N - 1) || (b - a) <= 1);

    // copy the chosen subsequence from the first parent to the offspring
    std::vector<bool> has(N, false);
    auto* offspring = new std::vector<int>(N);
    for (int i = a; i <= b; i++) {
        offspring->at(i) = parent1[i];
        has[offspring->at(i)] = true;
    }

    // copy the remaining elements keeping the relative order that they appear in the second parent
    int x = 0;
    for (int i = 0; i < a; i++) {
        while (has[parent2[x]])  // pula os elementos que já foram copiados do primeiro pai
            x++;

        offspring->at(i) = parent2[x];
        x++;
    }
    for (int i = b + 1; i < N; i++) {
        while (has[parent2[x]])  // pula os elementos que já foram copiados do primeiro pai
            x++;

        offspring->at(i) = parent2[x];
        x++;
    }

    return offspring;
}

void GeneticAlgorithm::survivalSelection(std::vector<Solution*>* solutions, int Mi) {
    std::vector<double> biasedFitness = getBiasedFitness(solutions);
    for (int i = 0; i < solutions->size(); i++) {
        solutions->at(i)->id = i;
    }

    // look for clones and give them a very low biased fitness
    // this way they will be removed from population
    const double INF = data.V * 10;
    std::vector<bool> isClone(solutions->size(), false);
    for (int i = 0; i < solutions->size(); i++) {
        if (isClone[i]) continue;
        for (int j = i + 1; j < solutions->size(); j++) {
            if (solutions->at(i)->equals(solutions->at(j))) {
                isClone[j] = true;
                biasedFitness[j] += INF;
            }
        }
    }

    // sort the solutions based on the biased fitness and keep only the best 'mu' solutions
    sort(solutions->begin(), solutions->end(),
         [&biasedFitness](Solution* s1, Solution* s2) { return biasedFitness[s1->id] < biasedFitness[s2->id]; });
    for (int c = Mi + 1; c < solutions->size(); c++) {
        delete solutions->at(c);
    }
    solutions->resize(Mi);
}

void GeneticAlgorithm::diversify(std::vector<Solution*>* solutions) {
    survivalSelection(solutions, data.params.mu / 3);  // keeps the mi/3 best solutions we have so far

    // generate more solutions with the same procedure that generated the initial population
    // and appends to the current solutions
    std::vector<Sequence*>* population = initializePopulation();

    for (auto sequence : *population) {
        auto* s = new Solution(data, *sequence);
        if (s->time < this->bestSolution->time) {
            delete this->bestSolution;
            this->bestSolution = s->copy();
            searchProgress.emplace_back(data.elapsedTime(), s->time);
        }
        solutions->push_back(s);
        delete sequence;
    }
    delete population;
}
