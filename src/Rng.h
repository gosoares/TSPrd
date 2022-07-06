#ifndef TSPRD_RNG_H
#define TSPRD_RNG_H

#include <functional>
#include <random>

using namespace std;

class Rng {
   private:
    static long long seed;
    static mt19937* generator;

   public:
    static void initialize(long long seed) {
        Rng::seed = seed;
        generator = new mt19937(seed);
    }

    static function<int()> getIntGenerator(int from, int to) {
        uniform_int_distribution<int> dist(from, to);
        return bind(dist, *generator);
    }

    static mt19937 getGenerator() { return *Rng::generator; }

    static random_device::result_type getSeed() { return Rng::seed; }
};

#endif  // TSPRD_RNG_H
