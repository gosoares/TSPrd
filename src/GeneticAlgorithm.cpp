#include "GeneticAlgorithm.h"

GeneticAlgorithm::GeneticAlgorithm(Data& data) : data(data), split(data), localSearch(data), population(data, split) {
    std::chrono::milliseconds maxTime(this->data.params.timeLimit * 1000);

    population.initialize();

    int itNotImproved = 0;
    while (itNotImproved < this->data.params.itNi && data.elapsedTime() < maxTime) {
        auto offspring = orderCrossover();
        split.split(offspring);

        localSearch.educate(*offspring);

        bool improvedBest = population.add(offspring);

        // Check if the population reached the maximum size
        if (population.size() > data.params.mu + data.params.lambda) {
            population.survivorsSelection();
        }

        if (improvedBest) {
            itNotImproved = 0;
        } else {
            itNotImproved++;
            if (itNotImproved % data.params.itDiv == 0) population.diversify();
        }
    }
}

Individual* GeneticAlgorithm::orderCrossover() {
    auto [parent1, parent2] = population.selectParents();
    auto offspring = new Individual(data.N);

    std::uniform_int_distribution<int> dist(0, data.N - 1);
    std::vector<bool> has(data.V, false);  // mark which vertices are already in the offstring
    int start = dist(data.generator), end;
    do {
        end = dist(data.generator);
    } while (start == end);

    for (int pos = start; pos != end; pos = (pos + 1) % data.N) {
        offspring->giantTour[pos] = parent1->giantTour[pos];
        has[parent1->giantTour[pos]] = true;
    }

    // copy the remaining elements keeping the relative order that they appear in the second parent
    for (int pos = end, j = 0; pos != start; pos = (pos + 1) % data.N) {
        while (has[parent2->giantTour[j]]) j++;

        offspring->giantTour[pos] = parent2->giantTour[j];
        j++;
    }

    return offspring;
}
