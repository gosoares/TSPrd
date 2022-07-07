#ifndef TSPRD_RNG_H
#define TSPRD_RNG_H

#include <random>

using namespace std;

class Rng {
   private:
    static long long seed;
    static mt19937 generator;

   public:
    static void initialize(long long seed) {
        Rng::seed = seed;
        generator.seed(seed);
    }

    static mt19937& getGenerator() { return Rng::generator; }

    static random_device::result_type getSeed() { return Rng::seed; }
};

#endif  // TSPRD_RNG_H
