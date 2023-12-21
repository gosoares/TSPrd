#include "Data.h"

Data::Data(const Instance& instance, const AlgParams& params)
    : V(instance.V), N(V - 1), timesMatrix(instance.timesMatrix), releaseDates(instance.releaseDates),
      biggerReleaseDate(*max_element(releaseDates.begin(), releaseDates.end())), symmetric(instance.symmetric),
      params(params), startTime(std::chrono::steady_clock::now()), generator(params.seed) {}

std::chrono::milliseconds Data::elapsedTime() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
}

std::tuple<std::string, std::string, AlgParams> Data::parseArgs(int argc, char** argv) {
    argparse::ArgumentParser program("TSPrd");
    program.add_argument("instanceFile").help("Instance file");

    program.add_argument("-t", "--timeLimit")
        .help("Maximum execution time, in seconds")
        .default_value(600)
        .scan<'i', int>();

    program.add_argument("-o", "--outputFile").help("File to print the execution results").default_value("");

    program.add_argument("-s", "--seed")
        .help("Numeric value for seeding the RNG")
        .default_value(((int)std::random_device{}()))
        .scan<'i', int>();

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
    int seed = program.get<int>("--seed");

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
                     .seed = seed};

    return std::make_tuple(instanceFile, outputFile, params);
}
