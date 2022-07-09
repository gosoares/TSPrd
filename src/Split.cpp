#include "Split.h"

Split::Split(Data& data) : data(data), rdPos(data.N), cumulative(data.N), bestIn(data.N), phi(data.N) {}

void Split::split(Individual* indiv) {
    load(indiv);

    fill(phi.begin(), phi.end(), INF);

    for (int j = 0; j < data.N; j++) {
        int rdPosJ = rdPos[j];
        int rdj = data.releaseDates[indiv->giantTour[rdPosJ]];
        int jToDepot = data.timesMatrix[indiv->giantTour[j]][0];
        int cumulativeJ = cumulative[j];

        for (int i = 0; i <= rdPosJ; i++) {
            int sigma = rdj;
            if (i > 0) sigma = std::max(sigma, phi[i - 1]);

            int deltaJ = sigma + data.timesMatrix[0][indiv->giantTour[i]] + (cumulativeJ - cumulative[i]) + jToDepot;
            if (deltaJ < phi[j]) {
                phi[j] = deltaJ;
                bestIn[j] = i;
            }
        }
    }

    save(indiv);
}

void Split::load(Individual* indiv) {
    rdPos[0] = 0;
    cumulative[0] = 0;  // cumulative of the arcs times till vertex in position i

    // calculate the position of the vertices with increasing release dates
    // and the cumulative times
    for (int i = 1; i < data.N; i++) {
        if (data.releaseDates[indiv->giantTour[i]] > data.releaseDates[indiv->giantTour[rdPos[i - 1]]]) {
            rdPos[i] = i;
        } else {
            rdPos[i] = rdPos[i - 1];
        }
        cumulative[i] = cumulative[i - 1] + data.timesMatrix[indiv->giantTour[i - 1]][indiv->giantTour[i]];
    }
}

void Split::save(Individual* indiv) {
    int i = data.N - 1;
    indiv->predecessors[0] = indiv->giantTour[i];  // predecessor of depot is the last client
    indiv->successors[i] = indiv->giantTour[0];    // successor of depot is the first client
    indiv->predecessors[indiv->giantTour[0]] = 0;
    indiv->successors[indiv->giantTour[i]] = 0;

    int nextDepot = bestIn[i] - 1;
    for (i = i - 1; i >= 0; i--) {
        if (i == nextDepot) {
            indiv->predecessors[indiv->giantTour[i + 1]] = 0;
            indiv->successors[indiv->giantTour[i]] = 0;
            nextDepot = bestIn[i] - 1;
        } else {
            indiv->predecessors[indiv->giantTour[i + 1]] = indiv->giantTour[i];
            indiv->successors[indiv->giantTour[i]] = indiv->giantTour[i + 1];
        }
    }

    indiv->eval = phi.back();
}
