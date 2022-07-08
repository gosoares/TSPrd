#include "GeneticAlgorithm.h"

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
