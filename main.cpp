#include <iostream>
#include "Instance.h"
#include "Solution.h"
#include "MathModel.h"
#include <algorithm>
#include <random>
#include <chrono>

using namespace std;

int main() {

    string instanceFile = "instance.dat";
    Instance instance = Instance(instanceFile);

    vector<unsigned int> v({1, 3, 4, 2});

//    vector<unsigned int> v(instance.nClients() - 1);
//    for (int i = 0; i < v.size(); i++) {
//        v[i] = i + 1;
//    }
//    shuffle(v.begin() + 1, v.end(), std::default_random_engine(seed));

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    MathModel(instance, v);
    auto s = Solution(instance, v);

    chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    cout << "Time difference = " << (chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) << "[µs]" << endl;


    return 0;
}


