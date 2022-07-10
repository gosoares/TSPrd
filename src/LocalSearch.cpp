#include "LocalSearch.h"

LocalSearch::LocalSearch(Data& data)
    : data(data), clients(), routesObj(data.V + 1, {data}), routes(data.V + 1), routesCoupling(data.N, {data.N}),
      intraMovesOrder(N_INTRA), interMovesOrder(N_INTER) {
    routes.resize(0);  // resize but keep allocated space
    clients.reserve(data.V);
    for (int c = 0; c < data.V; c++) clients.emplace_back(data, c);

    std::iota(intraMovesOrder.begin(), intraMovesOrder.end(), 1);
    std::iota(interMovesOrder.begin(), interMovesOrder.end(), 1);
}

void LocalSearch::educate(Individual& indiv) {
    load(indiv);
    intraSearch();
    interSearch();
    saveTo(indiv);
}

/*
********************** INTRA SEARCH FUNCTIONS *****************************
*/

bool LocalSearch::intraSearch() {
    std::shuffle(intraMovesOrder.begin(), intraMovesOrder.end(), data.generator);
    bool improvedAny = false, improved;

    for (auto route : routes) {
        route1 = route;

        whichMove = 0;
        while (whichMove < N_INTRA) {
            move = intraMovesOrder[whichMove];
            improved = callIntraSearch();

            if (improved) {
                std::shuffle(intraMovesOrder.begin(), intraMovesOrder.end(), data.generator);
                whichMove = 0;
                improvedAny = true;
            } else {
                whichMove++;
            }
        }
    }

    updateRoutesData();
    return improvedAny;
}

bool LocalSearch::callIntraSearch() {
    if (move == 1) {  // swap 1 1
        b1Size = 1, b2Size = 1;
        return intraSwap();
    } else if (move == 2) {  // swap 1 2
        b1Size = 1, b2Size = 2;
        return intraSwap();
    } else if (move == 3) {  // swap 2 2
        b1Size = 2, b2Size = 2;
        return intraSwap();
    } else if (move == 4) {  // relocation 1
        b1Size = 1;
        return intraRelocation();
    } else if (move == 5) {  // relocation 2
        b1Size = 2;
        return intraRelocation();
    }

    throw "Intra move id not known: " + whichMove;
}

void printRoute(Node* node) {
    std::cout << 0;
    int totalTime = 0, time;
    while (node != nullptr) {
        time = node->prev->timeTo[node->id];
        totalTime += time;
        std::cout << " -(" << time << ")> " << node->id;
        node = node->next;
    }
    std::cout << " T = " << totalTime << std::endl << std::endl;
}

bool LocalSearch::intraSwap() {
    if (route1->nClients < b1Size + b2Size) return false;  // not enough clients to make blocks of these sizes

    bestImprovement = 0;
    intraSwapOneWay();
    if (b1Size != b2Size) {
        std::swap(b1Size, b2Size);
        intraSwapOneWay();
    }

    if (bestImprovement > 0) {  // found an improvement
        b1Start = bestB1Start, b2Start = bestB2Start, b1End = bestB1End, b2End = bestB2End;
        swapBlocks();
        return true;
    }
    return false;
}

void LocalSearch::intraSwapOneWay() {
    for (resetBlock1(); !blocksFinished; moveBlock1Forward()) {
        preMinus = b1Start->prev->timeTo[b1Start->id]  // before b1
                   + b1End->timeTo[b1End->next->id];   // after b1

        for (resetBlock2Intra(); b2End->id != 0; moveBlock2Forward()) {
            minus = preMinus + b2End->timeTo[b2End->next->id];  // after b2
            plus = b1Start->prev->timeTo[b2Start->id]           // new arc before b2
                   + b1End->timeTo[b2End->next->id];            // new arc after b1

            if (b1End->next == b2Start) {            // adjacent
                plus += b2End->timeTo[b1Start->id];  // after new b2 == before new b1
            } else {
                minus += b2Start->prev->timeTo[b2Start->id];  // old arc before b2
                plus += b2Start->prev->timeTo[b1Start->id]    // new arc before b1
                        + b2End->timeTo[b1End->next->id];     // new arc after b2
            }

            improvement = minus - plus;
            evaluateImprovement();
        }
    }
}

bool LocalSearch::intraRelocation() {
    bestImprovement = 0;
    for (resetBlock1(); b1End->id != 0; moveBlock1Forward()) {
        posNode = &(route1->begin);  // position to try relocate to

        preMinus = b1Start->prev->timeTo[b1Start->id] + b1End->timeTo[b1End->next->id];
        prePlus = b1Start->prev->timeTo[b1End->next->id];

        while (posNode->id != 0) {
            if (posNode->next == b1Start) posNode = b1End->next;  // skip the block

            minus = preMinus + posNode->timeTo[posNode->next->id];
            plus = prePlus + posNode->prev->timeTo[b1Start->id] + b1End->timeTo[posNode->next->id];
            posNode = posNode->next;

            improvement = minus - plus;
            evaluateImprovement();
        }
    }

    if (bestImprovement > 0) {  // found a improvement
        relocateBlock();
        return true;
    }
    return false;
}

bool LocalSearch::interSearch() {
    std::shuffle(interMovesOrder.begin(), interMovesOrder.end(), data.generator);
    bool improvedAnyRoute, improvedAny = false;

    whichMove = 0;
    while (whichMove < N_INTER) {
        move = interMovesOrder[whichMove];

        improvedAnyRoute = false;
        for (int r2 = 1; r2 < routes.size() && !improvedAnyRoute; r2++) {
            route2 = routes[r2];

            for (int r1 = r2 - 1; r1 >= 0 && !improvedAnyRoute; r1++) {
                route1 = routes[r1];

                improvedAnyRoute = callInterSearch();
            }
        }

        if (improvedAnyRoute) {
            std::shuffle(intraMovesOrder.begin(), intraMovesOrder.end(), data.generator);
            whichMove = 0;
            improvedAny = true;
        } else {
            whichMove++;
        }
    }

    return improvedAny;
}

bool LocalSearch::callInterSearch() {
    if (whichMove == 1) return false;

    throw "Inter move id not known: " + whichMove;
}

/*
********************** OTHER FUNCTIONS *****************************
*/

void LocalSearch::resetBlock1() {
    blocksFinished = false;

    b1Start = route1->begin.next;
    b1End = b1Start;
    for (i = 1; i < b1Size; i++) b1End = b1End->next;
}

void LocalSearch::resetBlock2Intra() {
    b2Start = b1End->next;
    b2End = b2Start;
    for (i = 1; i < b2Size; i++) b2End = b2End->next;

    if (b2End->id == 0) {
        // not possible position for block2 after block1 in that route
        blocksFinished = true;
        return;
    }
}

void LocalSearch::resetBlock2Inter() {
    b2Start = route2->begin.next;
    b2End = b2Start;
    for (i = 1; i < b2Size; i++) b2End = b2End->next;
}

void LocalSearch::moveBlock1Forward() {
    b1Start = b1Start->next;
    b1End = b1End->next;
}

void LocalSearch::moveBlock2Forward() {
    b2Start = b2Start->next;
    b2End = b2End->next;
}

void LocalSearch::swapBlocks() {
    b1Start->prev->next = b2Start;
    aux = b1Start->prev;
    b1Start->prev = b2Start->prev;

    b2Start->prev->next = b1Start;
    b2Start->prev = aux;

    b1End->next->prev = b2End;
    aux = b1End->next;
    b1End->next = b2End->next;

    b2End->next->prev = b1End;
    b2End->next = aux;
}

void LocalSearch::relocateBlock() {
    b1Start->prev->next = b2End->next;
    b2End->next->prev = b1Start->prev;

    aux = posNode->next;
    posNode->next->prev = b2End;
    posNode->next = b1Start;

    b1Start->prev = posNode;
    b1End->next = aux;
}

void LocalSearch::evaluateImprovement() {
    if (improvement > bestImprovement) {
        bestImprovement = improvement;
        bestB1Start = b1Start;
        bestB1End = b1End;
        bestB2Start = b2Start;
        bestB2End = b2End;
    }
}

void LocalSearch::updateRoutesData() {
    int prevEnd = 0;

    for (auto route : routes) {
        route->nClients = -1;

        // fill forward data
        Node* node = route->begin.next;
        while (node != nullptr) {
            node->durationBefore = node->prev->durationBefore + node->prev->timeTo[node->id];
            node->predecessorsRd = std::max<int>(node->prev->predecessorsRd, node->prev->releaseDate);
            node = node->next;
            route->nClients++;
        }

        // fill backward data
        node = route->end.prev;
        while (node != nullptr) {
            node->durationAfter = node->next->durationAfter + node->timeTo[node->next->id];
            node->successorsRd = std::max<int>(node->next->successorsRd, node->next->releaseDate);
            node = node->prev;
        }

        route->releaseDate = route->end.predecessorsRd;
        route->duration = route->end.durationBefore;
        route->startTime = std::max<int>(route->releaseDate, prevEnd);
        route->endTime = route->startTime + route->duration;
        prevEnd = route->endTime;
    }

    // calculate the coupling between each pair of routes
    // first between adjacent routes
    // int R = routes.size() - 1;
    // for (int r = 0; r < R; r++) {
    //     routesCoupling[r][r] = INF;
    //     routesCoupling[r][r + 1] = (routes[r]->startingTime + routes[r]->duration) - (routes[r + 1]->releaseDate);
    // }
    // routesCoupling[R][R] = INF;

    // int couplingTotal = INF, couplingNext;
    // for (int r2 = routes.size() - 1; r2 >= 1; r2--) {
    //     for (int r1 = r2 - 2; r1 >= 0; r1--) {
    //         couplingNext = routesCoupling[r1][r1 + 1];
    //         couplingTotal = std::min<int>(couplingTotal, couplingNext, couplingTotal + couplingNext);
    //         routesCoupling[r1][r2] = couplingTotal;
    //     }
    // }
}

void LocalSearch::addRoute() {
    pos = routes.size();
    if (emptyRoutes.empty()) {
        routes.push_back(&(routesObj[routes.size()]));
    } else {
        routes.push_back(emptyRoutes.back());
        emptyRoutes.pop_back();
    }
    lastRoute = routes.back();
    lastRoute->pos = pos;
}

void LocalSearch::load(const Individual& indiv) {
    // create the routes
    routes.clear();
    emptyRoutes.clear();
    addRoute();
    node = &(lastRoute->begin);

    for (int i = 0; i < data.N; i++) {
        node->next = &(clients[indiv.giantTour[i]]);
        node->next->prev = node;
        node = node->next;

        if (indiv.successors[indiv.giantTour[i]] == 0) {
            node->next = &(lastRoute->end);
            lastRoute->end.prev = node;

            if (i + 1 < data.N) {
                addRoute();
                node = &(lastRoute->begin);
            }
        }
    }
    node->next = &(lastRoute->end);
    lastRoute->end.prev = node;

    updateRoutesData();
}

void LocalSearch::saveTo(Individual& indiv) {
    indiv.eval = routes.back()->endTime;

    indiv.predecessors[0] = routes.back()->end.prev->id;
    indiv.successors[0] = routes[0]->begin.next->id;

    int pos = 0;
    for (auto route : routes) {
        Node* node = route->begin.next;
        do {
            indiv.giantTour[pos++] = node->id;
            indiv.predecessors[node->id] = node->prev->id;
            indiv.successors[node->id] = node->next->id;

            node = node->next;
        } while (node->id != 0);
    }
}
