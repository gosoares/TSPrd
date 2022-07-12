#ifndef TSPRD_SPLIT_H
#define TSPRD_SPLIT_H

#include "Data.h"
#include "Individual.h"

class Split {
   private:
    Data& data;

    /*stores for each position in the big tour which of the previous
    positions has the first vertex with bigger release date*/
    std::vector<int> rdPos;

    // cumulative of arc times
    std::vector<int> cumulative;

    // durint split, stores the origin of the best arc arriving at i
    std::vector<int> bestIn;

    // during the split, stores the value of the best arc arriving at i
    std::vector<int> phi;

    void load(Individual* indiv);
    void save(Individual* indiv);

   public:
    Split(Data& data);

    void split(Individual* indiv);
};

#endif  // TSPRD_SPLIT_H
