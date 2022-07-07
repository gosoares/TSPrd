#include "Data.h"

Data::Data(const Instance& instance, const AlgParams& params)
    : V(instance.V), N(V - 1), timesMatrix(instance.timesMatrix), releaseDates(instance.releaseDates),
      biggerReleaseDate(*max_element(releaseDates.begin(), releaseDates.end())), symmetric(instance.symmetric),
      params(params), startTime(std::chrono::steady_clock::now()), generator(params.seed) {}

std::chrono::milliseconds Data::elapsedTime() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
}

void paramsError() {
    std::string usage =
        "usage: TSPrd instance_file [options]\n"
        "Possible options:\n"
        "-t timeLimit          Maximum execution time, in seconds\n"
        "-o outputFile         File to print the execution results\n"
        "-s seed               Numeric value for seeding the RNG\n";
    std::cout << usage << std::endl;
    exit(1);
}

std::tuple<std::string, std::string, AlgParams> Data::parseArgs(int argc, char** argv) {
    // default parameters
    int mu = 20;
    int lambda = 40;
    int nbElite = 8;
    int nClose = 6;
    int itNi = 10000;
    int itDiv = 4000;
    int timeLimit = 600;
    long long seed = std::random_device{}();

    if (argc < 2 || argc % 2 == 1) {
        paramsError();
    }

    std::string instanceFile = argv[1];
    std::string outputFile = "";

    if (!std::filesystem::exists(instanceFile)) {
        std::cout << "Not able to find this file: " << instanceFile << std::endl;
        exit(1);
    }

    for (int i = 2; i < argc; i += 2) {
        auto arg = std::string(argv[i]);

        if (arg == "-t")
            timeLimit = std::stoi(argv[i + 1]);
        else if (arg == "-o")
            outputFile = argv[i + 1];
        else if (arg == "-s")
            seed = std::stoll(argv[i + 1]);
        else {
            std::cout << "Unknown option: " + arg << std::endl;
            paramsError();
        }
    }

    AlgParams params{.mu = mu,
                     .lambda = lambda,
                     .nbElite = nbElite,
                     .nClose = nClose,
                     .itNi = itNi,
                     .itDiv = itDiv,
                     .timeLimit = timeLimit,
                     .seed = seed};

    return std::make_tuple(instanceFile, outputFile, params);
}
