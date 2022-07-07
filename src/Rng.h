#ifndef TSPRD_RNG_H
#define TSPRD_RNG_H

#include <random>

class Rng {
   private:
    static long long seed;
    static std::mt19937 generator;

   public:
    static std::mt19937& getGenerator() { return Rng::generator; }

    static std::random_device::result_type getSeed() { return Rng::seed; }
};

#endif  // TSPRD_RNG_H
