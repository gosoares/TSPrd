#ifndef TSPRD_DATA_H
#define TSPRD_DATA_H

#include <algorithm>
#include <argparse/argparse.hpp>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "Instance.h"

const int INF = std::numeric_limits<int>::max() / 2;

// parameters for the algorithm
struct AlgParams {
    int mu;         // minimum size of the population
    int lambda;     // maximum number of additional individuals in the population
    int nbElite;    // number of elite individuals for the biased fitness
    int nClose;     // number of closest individuals to consider when calculating the diversity
    int itNi;       // max iterations without improvement to stop the algorithm
    int itDiv;      // iterations without improvement to diversify
    int timeLimit;  // in seconds

    unsigned int seed;  // seed for RNG

    std::vector<std::string> intraMoves;  // list of intra-route moves
    std::vector<std::string> interMoves;  // list of inter-route moves
};

class Data {
   public:
    // instance data
    const int V, N;                                    // how many vertices and how many clients (V-1 )
    const std::vector<std::vector<int>>& timesMatrix;  // times matrix
    const std::vector<int>& releaseDates;              // release date of each vertex
    const int biggerReleaseDate;                       // the bigger release date in the instance
    const bool symmetric;                              // whether the time matrix is symmetric

    // parameters for the algorithm
    const AlgParams& params;

    // starting time of the algorithm
    const std::chrono::steady_clock::time_point startTime;

    // for RNG
    std::mt19937 generator;

    explicit Data(const Instance& instance, const AlgParams& params)
        : V(instance.V), N(V - 1), timesMatrix(instance.timesMatrix), releaseDates(instance.releaseDates),
          biggerReleaseDate(*max_element(releaseDates.begin(), releaseDates.end())), symmetric(instance.symmetric),
          params(params), startTime(std::chrono::steady_clock::now()), generator(params.seed) {}

    std::chrono::milliseconds elapsedTime() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
    }

    static std::tuple<std::string, std::string, AlgParams> parseArgs(int argc, char** argv) {
        argparse::ArgumentParser program("TSPrd");
        program.add_argument("instanceFile").help("Instance file");

        program.add_argument("-t", "--timeLimit")
            .help("Maximum execution time, in seconds")
            .default_value(600)
            .scan<'i', int>();

        program.add_argument("-o", "--outputFile").help("File to print the execution results").default_value("");

        program.add_argument("-s", "--seed")
            .help("Numeric value for seeding the RNG")
            .default_value(std::random_device{}())
            .scan<'u', unsigned int>();

        program.add_argument("--intraMoves")
            .help("List of intra moves to be used")
            .nargs(argparse::nargs_pattern::any)
            .default_value(
                std::vector<std::string>{"swap11", "swap12", "swap22", "reinsertion1", "reinsertion2", "2opt"});
        program.add_argument("--interMoves")
            .help("List of inter moves to be used")
            .nargs(argparse::nargs_pattern::any)
            .default_value(std::vector<std::string>{"relocation", "swap", "divideAndSwap"});

        program.add_argument("--mu").help("Minimum size of the population").default_value(20).scan<'i', int>();
        program.add_argument("--lambda")
            .help("Maximum number of additional individuals in the population")
            .default_value(40)
            .scan<'i', int>();
        program.add_argument("--nbElite")
            .help("Number of elite individuals for the biased fitness")
            .default_value(8)
            .scan<'i', int>();
        program.add_argument("--nClose")
            .help("Number of closest individuals to consider when calculating the diversity")
            .default_value(6)
            .scan<'i', int>();
        program.add_argument("--itNi")
            .help("Max iterations without improvement to stop the algorithm")
            .default_value(10000)
            .scan<'i', int>();
        program.add_argument("--itDiv")
            .help("Iterations without improvement to diversify")
            .default_value(4000)
            .scan<'i', int>();

        try {
            program.parse_args(argc, argv);
        } catch (const std::runtime_error& err) {
            std::cout << "argparse error" << std::endl;
            std::cout << err.what() << std::endl;
            exit(1);
        }

        std::string instanceFile = program.get<std::string>("instanceFile");
        std::string outputFile = program.get<std::string>("--outputFile");
        int timeLimit = program.get<int>("--timeLimit");
        unsigned int seed = program.get<unsigned int>("--seed");

        auto intraMoves = program.get<std::vector<std::string>>("--intraMoves");
        auto interMoves = program.get<std::vector<std::string>>("--interMoves");

        int mu = program.get<int>("--mu");
        int lambda = program.get<int>("--lambda");
        int nbElite = program.get<int>("--nbElite");
        int nClose = program.get<int>("--nClose");
        int itNi = program.get<int>("--itNi");
        int itDiv = program.get<int>("--itDiv");

        if (!std::filesystem::exists(instanceFile)) {
            std::cout << "Not able to find this file: " << instanceFile << std::endl;
            exit(1);
        }

        AlgParams params{.mu = mu,
                         .lambda = lambda,
                         .nbElite = nbElite,
                         .nClose = nClose,
                         .itNi = itNi,
                         .itDiv = itDiv,
                         .timeLimit = timeLimit,
                         .seed = seed,
                         .intraMoves = intraMoves,
                         .interMoves = interMoves};

        return std::make_tuple(instanceFile, outputFile, params);
    }
};

#endif  // TSPRD_DATA_H
