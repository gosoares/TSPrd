#include <iostream>
#include "Instance.h"
#include "Solution.h"
#include "SplitMathModel.h"
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

int main() {

    string instanceFile = "instance.dat";
    Instance instance = Instance(instanceFile);

//    vector<unsigned int> v({1, 4, 3, 8, 5, 6, 7, 9, 2});

    vector<unsigned int> v(instance.nVertex() - 1);
    for (int i = 0; i < v.size(); i++) {
        v[i] = i + 1;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    shuffle(v.begin() + 1, v.end(), std::default_random_engine(seed));

    cout << endl << "Sequencia: " << v[0];
    for (int i = 1; i < v.size(); i++) {
        cout << " -> " << v[i];
    }
    cout << endl << endl;

    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    SplitMathModel(instance, v);
    auto s = Solution(instance, v);
    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cout << "Time difference = " << (chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) << "[µs]" << endl;

//    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//    int n = 5;
//    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//    for (int i = 0; i < n; i++) {
//        shuffle(v.begin() + 1, v.end(), std::default_random_engine(seed));
//        auto s = Solution(instance, v);
//    }
//    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//    cout << "Execution Time = " << (chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / n) << "[µs]" << endl;

    return 0;
}


