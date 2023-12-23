#ifndef TSPRD_INTERSEARCHALGO_H
#define TSPRD_INTERSEARCHALGO_H

#include <vector>

#include "DivideAndSwap.hpp"
#include "InterRelocation.hpp"
#include "InterSearch.hpp"
#include "InterSwap.hpp"

class InterSearchAlgo {
   private:
    std::vector<InterSearch*> searches;
    std::mt19937 generator;

    void shuffleSearches() { shuffle(searches.begin(), searches.end(), generator); }

    std::vector<InterSearch*> parseNames(std::vector<std::string> names, Data& data) {
        std::vector<InterSearch*> searches;
        for (auto& name : names) {
            if (name == "relocation") {
                searches.push_back(new InterRelocation(data));
            } else if (name == "swap") {
                searches.push_back(new InterSwap(data));
            } else if (name == "divideAndSwap") {
                searches.push_back(new DivideAndSwap(data));
            } else {
                std::cerr << "Invalid inter-route search name: " << name << std::endl;
                exit(1);
            }
        }

        return searches;
    }

   public:
    InterSearchAlgo(Data& data) : searches(parseNames(data.params.interMoves, data)), generator(data.generator) {}

    int search(Solution* solution) {
        if (solution->routes.size() == 1) return 0;

        int oldTime = solution->time;
        shuffleSearches();

        int whichSearch = 0;
        bool improved = false;
        while (whichSearch < searches.size()) {
            int gain = searches[whichSearch]->search(solution);

            if (gain > 0) {
                improved = true;
                if (solution->routes.size() == 1) break;
                auto& lastMovement = searches[whichSearch];
                shuffleSearches();
                whichSearch = 0;
                if (searches[0] == lastMovement) {
                    std::swap(searches[0], searches[searches.size() - 1]);
                }
            } else {
                whichSearch++;
            }
        }

        if (improved) {
            int newTime = solution->update();
            return oldTime - newTime;
        }

        return 0;
    }
};

#endif  // TSPRD_INTERSEARCHALGO_H
