#include "Data.h"
#include "GeneticAlgorithm.h"
#include "Instance.h"
#include "Solution.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: TSPrd instance_file [output_file] [seed]" << std::endl;
        exit(1);
    }

    long long seed = std::random_device{}();
    if (argc >= 4) seed = std::stoll(argv[3]);

    AlgParams params{.mu = 20,
                     .lambda = 40,
                     .nbElite = 8,
                     .nClose = 6,
                     .itNi = 10000,
                     .itDiv = 4000,
                     .timeLimit = 600,
                     .seed = seed};

    std::string instanceName = argv[1];
    Instance instance(instanceName);

    auto data = Data(instance, params);

    auto alg = GeneticAlgorithm(data);
    Solution s = alg.getSolution();
    s.validate();

    std::cout << "RESULT " << s.time << std::endl;
    std::cout << "EXEC_TIME " << alg.getExecutionTime() << std::endl;
    std::cout << "SOL_TIME " << alg.getBestSolutionTime() << std::endl;
    std::cout << "SEED " << params.seed << std::endl;

    if (argc < 3) return 0;

    // output to file
    std::string outFile = std::string(argv[2]);
    std::string dir = outFile.substr(0, outFile.find_last_of('/'));
    if (dir != outFile) std::filesystem::create_directories(dir);  // make sure the path exists

    std::ofstream fout(outFile, std::ios::out);
    fout << "EXEC_TIME " << alg.getExecutionTime() << std::endl;
    fout << "SOL_TIME " << alg.getBestSolutionTime() << std::endl;
    fout << "OBJ " << s.time << std::endl;
    fout << "SEED " << params.seed << std::endl;
    fout << "N_ROUTES " << s.routes.size() << std::endl;
    fout << "N_CLIENTS";
    for (auto& r : s.routes) fout << " " << (r->size() - 2);
    fout << std::endl << "ROUTES" << std::endl;
    for (auto& r : s.routes) {
        for (unsigned int c = 1; c < r->size() - 1; c++) {
            fout << r->at(c) << " ";
        }
        fout << std::endl;
    }
    fout << std::endl;
    fout.close();

    // output search progress
    auto dotPos = outFile.find_last_of('.');
    std::string spFile = outFile.substr(0, dotPos) + "_SP" + outFile.substr(dotPos);
    std::ofstream spout(spFile, std::ios::out);
    spout << "time,obj" << std::endl;
    for (auto x : alg.getSearchProgress()) {
        spout << std::chrono::duration_cast<std::chrono::milliseconds>(x.first - data.startTime).count() << ","
              << x.second << std::endl;
    }
    spout.close();
    return 0;
}
