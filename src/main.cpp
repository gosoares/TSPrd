#include "Data.h"
#include "GeneticAlgorithm.h"
#include "Instance.h"
#include "Solution.h"

int main(int argc, char** argv) {
    auto [instanceName, outputFile, params] = Data::parseArgs(argc, argv);
    auto instance = Instance(instanceName);
    auto data = Data(instance, params);

    auto alg = GeneticAlgorithm(data);
    auto endTime = std::chrono::steady_clock::now();
    auto execTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - data.startTime).count();

    std::cout << "EXEC_TIME " << execTime << std::endl;
    std::cout << "SOL_TIME " << alg.population.searchProgress.back().first << std::endl;
    std::cout << "OBJ " << alg.population.bestSolution.eval << std::endl;
    std::cout << "SEED " << params.seed << std::endl;

    if (outputFile == "") return 0;

    // output to file
    std::string dir = outputFile.substr(0, outputFile.find_last_of('/'));
    // if (dir != outputFile) std::filesystem::create_directories(dir);  // make sure the path exists

    std::ofstream fout(outputFile, std::ios::out);
    fout << "EXEC_TIME " << execTime << std::endl;
    fout << "SOL_TIME " << alg.population.searchProgress.back().first << std::endl;
    fout << "OBJ " << alg.population.bestSolution.eval << std::endl;
    fout << "SEED " << params.seed << std::endl;

    auto s = Solution(data, alg.population.bestSolution.giantTour, nullptr);
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

    // output search progress
    // auto dotPos = outputFile.find_last_of('.');
    // std::string spFile = outputFile.substr(0, dotPos) + "_SP" + outputFile.substr(dotPos);
    // std::ofstream spout(spFile, std::ios::out);
    // spout << "time,obj" << std::endl;
    // for (auto x : alg.getSearchProgress()) {
    //     spout << std::chrono::duration_cast<std::chrono::milliseconds>(x.first - data.startTime).count() << ","
    //           << x.second << std::endl;
    // }
    // spout.close();
    return 0;
}
