#include "LocalSearch.h"

bool LocalSearch::interSearch() {
    updateRoutesData();

    std::shuffle(interMovesOrder.begin(), interMovesOrder.end(), data.generator);
    bool improvedAnyRoute, improvedAny = false;

    whichMove = 0;
    while (whichMove < N_INTER) {
        move = interMovesOrder[whichMove];
        improvedAnyRoute = false;

        for (rx = 1; rx < routes.size() && !improvedAnyRoute; rx++) {
            for (ry = 0; ry < rx && !improvedAnyRoute; ry++) {
                r1.pos = ry, r2.pos = rx;
                improvedAnyRoute = callInterSearch();
            }

            for (ry = 0; ry < rx && !improvedAnyRoute; ry++) {
                r1.pos = rx, r2.pos = ry;
                improvedAnyRoute = callInterSearch();
            }
        }

        if (improvedAnyRoute) {
            std::shuffle(interMovesOrder.begin(), interMovesOrder.end(), data.generator);
            whichMove = 0;
            improvedAny = true;
            updateRoutesData();
        } else {
            whichMove++;
        }
    }

    return improvedAny;
}

bool LocalSearch::callInterSearch() {
    r1.route = routes[r1.pos];
    r2.route = routes[r2.pos];
    r1.endBefore = r1.pos == 0 ? 0 : routes[r1.pos - 1]->endTime;
    r2.endBefore = r2.pos == 0 ? 0 : routes[r2.pos - 1]->endTime;

    if (move == 1) {  // relocation 1
        b1Size = 1;
        return interRelocation();
    } else if (move == 2) {  // relocation 2
        b1Size = 2;
        return interRelocation();
    } else if (move == 3) {  // swap 1 1
        if (r1.pos > r2.pos) return false;
        b1Size = 1, b2Size = 1;
        return interSwap();
    } else if (move == 4) {  // swap 1 2
        b1Size = 1, b2Size = 2;
        return interSwap();
    } else if (move == 5) {  // swap 2 2
        if (r1.pos > r2.pos) return false;
        b1Size = 2, b2Size = 2;
        return interSwap();
    }

    throw "Inter move id not known: " + move;
}

bool LocalSearch::interRelocation() {
    for (resetBlock1(); !blocks1Finished; moveBlock1Forward()) {
        r1.newReleaseDate = std::max<int>(b1->predecessorsRd, b1End->successorsRd);
        r1.newDuration = b1->prev->durationBefore + b1->prev->timeTo[b1End->next->id] + b1End->next->durationAfter;
        r2.newReleaseDate = std::max<int>(r2.route->releaseDate, b1ReleaseDate);

        for (b2 = &(r2.route->begin); b2->next != nullptr; b2 = b2->next) {
            r2.newDuration = b2->durationBefore + b2->timeTo[b1->id] + b1Duration + b1End->timeTo[b2->next->id] +
                             b2->next->durationAfter;

            if (evaluateInterRouteImprovement()) {
                relocateBlock();
                checkEmptyRoute(r1.route);
                return true;
            }
        }
    }

    return false;
}

bool LocalSearch::interSwap() {
    for (resetBlock1(); !blocks1Finished; moveBlock1Forward()) {
        preR1ReleaseDate = std::max<int>(b1->predecessorsRd, b1End->successorsRd);  // rd removing block
        preR1Duration = b1->durationBefore + b1End->durationAfter;

        for (resetBlock2Inter(); !blocks2Finished; moveBlock2Forward()) {
            r1.newReleaseDate = std::max<int>(preR1ReleaseDate, b2->releaseDate);
            r1.newDuration = preR1Duration + b2Duration + b1->prev->timeTo[b2->id] + b2End->timeTo[b1End->next->id];

            r2.newReleaseDate = std::max<int>(std::max<int>(b2->predecessorsRd, b2->successorsRd), b1ReleaseDate);
            r2.newDuration = b2->durationBefore + b2->durationAfter + b1Duration + b2->prev->timeTo[b1->id] +
                             b1End->timeTo[b2->next->id];

            if (evaluateInterRouteImprovement()) {
                swapBlocks();
                checkEmptyRoute(r1.route);
                checkEmptyRoute(r2.route);
                return true;
            }
        }
    }
    return false;
}
