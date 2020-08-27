#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"
#pragma ide diagnostic ignored "cert-msc51-cpp"
#include <iostream>
#include "Instance.h"
#include "Solution.h"
#include "SplitMathModel.h"
#include "NeighborSearch.h"
#include "Functions.h"
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

int routeTime(const Instance& instance, const vector<unsigned int>& r) {
    cout << "Rota: " << r[0];
    int time = 0;
    for (int i = 1; i < r.size(); i++) {
        cout << " -> " << r[i];
        time += (int) instance.time(r[i-1], r[i]);
    }
    cout << endl << "Tempo: " << time << endl << endl;

    return time;
}
void printSequence(const vector<unsigned int> &s) {
    cout << "Sequencia: " << s[0];
    for (int i = 1; i < s.size(); i++) {
        cout << " -> " << s[i];
    }
    cout << endl;
}

void freePopulation(vector<Sequence *> *population) {
    for(auto p: *population) {
        delete p;
    }
    population->clear();
}

int *selectParents(vector<double> &biasedFitness, int mi) {
    // seleciona o primeiro pai
    int p[2];
    int p1a = rand() % mi, p1b = rand() % mi;
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


Solution *geneticAlgorithm(const string &instanceFile, int mi, int lambda, int nClose, double nbElite, int itNi, int itDiv) {
    srand(time(nullptr));

    Instance instance = Instance(instanceFile);

    Functions f(instance);
    NeighborSearch ns(instance);

    vector<unsigned int> r({0, 30, 16, 8, 0});
    ns.swapSearch(r, 1, 1);

    vector<Sequence *> *population = f.initializePopulation(mi, lambda);
    vector<Solution *> *solutions = Solution::solutionsFromSequences(instance, population, mi);
    auto *bestSolution = new Solution(vector<vector<unsigned int> >(), 999999);

    int iterations_not_improved = 0;

    while(iterations_not_improved < itNi) {
        vector<double> biasedFitness(solutions->size());
        Functions::getBiasedFitness(biasedFitness, solutions, nbElite, nClose);

        while(solutions->size() < mi + lambda) {
            // SELECAO DOS PARENTES PARA CROSSOVER
            int *p = selectParents(biasedFitness, mi);

            Sequence *child = Functions::orderCrossover(*population->at(p[0]), *population->at(p[1]));
            auto *solution = new Solution(instance, *child);
            delete child;

            // EDUCACAO
            ns.educate(solution);
            solutions->push_back(solution);

            if (solution->time < bestSolution->time) {
                delete bestSolution;
                bestSolution = solution->copy();
                // cout << "Best Solution Found: " << bestSolution->time << endl;

                iterations_not_improved = 0;
            } else {
                iterations_not_improved++;
                if(iterations_not_improved == itDiv) { // DIVERSIFICACAO
                    f.diversificate(solutions, mi, nbElite, nClose);
                }
            }
        }

        Functions::survivalSelection(solutions, mi, nbElite, nClose);

        //recalculate population
        freePopulation(population);
        for(Solution *s: *solutions) {
            population->push_back(s->getSequence());
        }
    }

    return bestSolution;
}

int main(int argc, char **argv) {
    int mi = 25;
    int lambda = 100;
    int nClose = 5;
    double nbElite = 0.4;
    int itNi = 200; // max iterations without improvement to stop the algorithm
    int itDiv = 100; // iterations without improvement to diversificate
    string instanceFile = argv[1];

    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    Solution *s = geneticAlgorithm(instanceFile, mi, lambda, nClose, nbElite, itNi, itDiv);
    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cout << "RESULT " << s->time << endl;
    cout << "TIME " << chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << endl;


    return 0;
}