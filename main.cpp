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

void swapn(int n1, int n2) {
    swap(n1, n2);
    cout << n1 << "  " << n2 << endl;
}

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

int main_test() {
    srand(time(nullptr));
    string instanceFile = "instance.dat";
    Instance instance = Instance(instanceFile);

//    vector<unsigned int> sequence1({1, 4, 3, 2});
//    vector<unsigned int> sequence2({4, 3, 2, 1});

    vector<unsigned int> sequence1(instance.nVertex() - 1);
    vector<unsigned int> sequence2(instance.nVertex() - 1);
    for (int i = 0; i < instance.nVertex() - 1; i++) {
        sequence1[i] = i+1;
        sequence2[i] = i+1;
    }
    auto rand = default_random_engine(chrono::system_clock::now().time_since_epoch().count());
    shuffle(sequence1.begin(), sequence1.end(), rand);
    shuffle(sequence2.begin(), sequence2.end(), rand);

    printSequence(sequence1);
    printSequence(sequence2);
//    printSequence(Functions::orderCrossover(sequence1, sequence2));


//    NeighborSearch ns(instance);
//    vector<unsigned int> route({0, 1, 2, 3, 4, 0});
//    routeTime(instance, route);
//    ns.twoOptSearch(route);
//    routeTime(instance, route);

//    Solution s1(instance, sequence1);
//    Solution s2(instance, sequence2);
//
//    cout << Functions::routesDistance(s1, s1) << endl;
//    cout << Functions::routesDistance(s2, s2) << endl;
//    cout << Functions::routesDistance(s1, s2) << endl;

}

void freePopulation(vector<Sequence *> *population) {
    for(auto p: *population) {
        delete p;
    }
    population->clear();
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
            // seleciona o primeiro pai
            int p1a = rand() % mi, p1b = rand() % mi;
            int p1 = p1a;
            if (biasedFitness[p1b] > biasedFitness[p1a])
                p1 = p1b;


            // seleciona segundo pai, diferente do primeiro
            int p2;
            do {
                int p2a = rand() % mi, p2b = rand() % mi;
                p2 = p2a;
                if (biasedFitness[p2b] > biasedFitness[p2a])
                    p2 = p2b;
            } while (p1 == p2);

            Sequence *child = Functions::orderCrossover(*population->at(p1), *population->at(p2));
            auto *solution = new Solution(instance, *child);
            ns.educate(solution);

            delete child;
            solutions->push_back(solution);

            if (solution->time < bestSolution->time) {
                delete bestSolution;
                bestSolution = solution->copy();
                // cout << "Best Solution Found: " << bestSolution->time << endl;

                iterations_not_improved = 0;
            } else {
                iterations_not_improved++;
                if(iterations_not_improved == itDiv) {
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
    int itDiv = 100; // iterations without improvement
    string instanceFile = argv[1];

    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    Solution *s = geneticAlgorithm(instanceFile, mi, lambda, nClose, nbElite, itNi, itDiv);
    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cout << "RESULT " << s->time << endl;
    cout << "TIME " << chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << endl;


//    vector<unsigned int> v({4, 1, 3, 2});
//
////    vector<unsigned int> v(instance.nVertex() - 1);
////    for (int i = 0; i < v.size(); i++) {
////        v[i] = i + 1;
////    }
////    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
////    shuffle(v.begin() + 1, v.end(), std::default_random_engine(seed));
//
//    cout << endl << "Sequencia: " << v[0];
//    for (int i = 1; i < v.size(); i++) {
//        cout << ", " << v[i];
//    }
//    cout << endl << endl;
//
//    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//    SplitMathModel(instance, v);
//    auto s = Solution(instance, v);
//    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//    cout << "Time difference = " << (chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) << "[µs]" << endl;
//
////    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
////    int n = 5;
////    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
////    for (int i = 0; i < n; i++) {
////        shuffle(v.begin() + 1, v.end(), std::default_random_engine(seed));
////        auto s = Solution(instance, v);
////    }
////    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
////    cout << "Execution Time = " << (chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / n) << "[µs]" << endl;

    return 0;
}