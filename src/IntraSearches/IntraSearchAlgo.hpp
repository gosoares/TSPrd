#ifndef TSPRD_INTRASEARCHALGO_H
#define TSPRD_INTRASEARCHALGO_H

#include <random>
#include <string>
#include <vector>

#include "../Solution.h"
#include "IntraReinsertion.hpp"
#include "IntraSwap.hpp"
#include "IntraTwoOpt.hpp"

#define F(R) 1                  // index of first client in a route
#define L(R) ((R)->size() - 2)  // index of last client in a route

class IntraSearchAlgo {
   private:
    std::vector<IntraSearch*> searches;
    std::mt19937 generator;

    void shuffleSearches() { shuffle(searches.begin(), searches.end(), generator); }

    std::vector<IntraSearch*> parseNames(std::vector<std::string> names,
                                         const std::vector<std::vector<int>>& timesMatrix) {
        std::vector<IntraSearch*> searches;
        for (auto& name : names) {
            if (name == "swap11") {
                searches.push_back(new IntraSwap(timesMatrix, 1, 1));
            } else if (name == "swap12") {
                searches.push_back(new IntraSwap(timesMatrix, 1, 2));
            } else if (name == "swap22") {
                searches.push_back(new IntraSwap(timesMatrix, 2, 2));
            } else if (name == "reinsertion1") {
                searches.push_back(new IntraReinsertion(timesMatrix, 1));
            } else if (name == "reinsertion2") {
                searches.push_back(new IntraReinsertion(timesMatrix, 2));
            } else if (name == "2opt") {
                searches.push_back(new IntraTwoOpt(timesMatrix));
            } else {
                std::cerr << "Invalid intra-route search name: " << name << std::endl;
                exit(1);
            }
        }

        return searches;
    }

   public:
    explicit IntraSearchAlgo(const Data& data)
        : searches(parseNames(data.params.intraMoves, data.timesMatrix)), generator(data.generator) {}

    int search(Solution* solution) {
        int oldTime = solution->time;
        shuffleSearches();

        for (auto& route : solution->routes) {
            int whichSearch = 0;

            while (whichSearch < searches.size()) {
                int gain = searches[whichSearch]->search(route);

                if (gain > 0) {  // reset search order
                    shuffleSearches();
                    whichSearch = 0;
                } else {
                    whichSearch++;
                }
            }
        }

        int newTime = solution->update();
        return oldTime - newTime;
    }
};

#endif  // TSPRD_INTRASEARCHALGO_H
