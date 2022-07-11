#include "LocalSearch.h"

bool LocalSearch::interSearch() {
    std::shuffle(interMovesOrder.begin(), interMovesOrder.end(), data.generator);
    bool improvedAnyRoute, improvedAny = false;

    updateRoutesOrder(false);

    whichMove = 0;
    while (whichMove < N_INTER) {
        move = interMovesOrder[whichMove];
        improvedAnyRoute = false;

        updateRoutesOrder();

        for (auto routesPair : routesOrder) {
            r1.pos = routesPair.first;
            r2.pos = routesPair.second;
            if (callInterSearch()) {
                improvedAnyRoute = true;
                break;
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

    switch (move) {
        case 1:  // relocation 1
            b1Size = 1;
            return interRelocation();
            break;

        case 2:  // swap 1 1
            if (r1.pos > r2.pos) return false;
            b1Size = 1, b2Size = 1;
            return interSwap();
            break;

        case 3:  // relocation 2
            b1Size = 2;
            return interRelocation();
            break;

        case 4:  // swap 1 2
            b1Size = 1, b2Size = 2;
            return interSwap();
            break;

        case 5:  // swap 2 2
            if (r1.pos > r2.pos) return false;
            b1Size = 2, b2Size = 2;
            return interSwap();
            break;
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
        preR1Duration = b1->prev->durationBefore + b1End->next->durationAfter;

        for (resetBlock2Inter(); !blocks2Finished; moveBlock2Forward()) {
            if (b1->id == 13 && b1End->id == 13 && b2->id == 11 && b2End->id == 6) {
                std::cout << "here" << std::endl;
            }

            r1.newReleaseDate = std::max<int>(preR1ReleaseDate, b2ReleaseDate);
            r1.newDuration = preR1Duration + b2Duration + b1->prev->timeTo[b2->id] + b2End->timeTo[b1End->next->id];

            r2.newReleaseDate = std::max<int>(std::max<int>(b2->predecessorsRd, b2End->successorsRd), b1ReleaseDate);
            r2.newDuration = b2->prev->durationBefore + b2End->next->durationAfter + b1Duration +
                             b2->prev->timeTo[b1->id] + b1End->timeTo[b2End->next->id];

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

void LocalSearch::updateRoutesOrder(bool shuffle) {
    if (nRoutesOrder != routes.size()) {
        routesOrder.resize(routes.size() * (routes.size() - 1));  // initialize routes order
        for (rx = 0, i = 0; rx < routes.size(); rx++) {
            for (ry = 0; ry < routes.size(); ry++) {
                if (rx == ry) continue;
                routesOrder[i++] = {rx, ry};
            }
        }
    }

    if (shuffle) std::shuffle(routesOrder.begin(), routesOrder.end(), data.generator);
}
