#include "LocalSearch.h"

LocalSearch::LocalSearch(Data& data, Split& split)
    : data(data), split(split), clients(), routesObj(data.V + 1, {data}), routes(data.V + 1), intraMovesOrder(N_INTRA),
      interMovesOrder(N_INTER) {
    routes.resize(0);  // resize but keep allocated space
    clients.reserve(data.V);
    for (int c = 0; c < data.V; c++) clients.emplace_back(data, c);

    std::iota(intraMovesOrder.begin(), intraMovesOrder.end(), 1);
    std::iota(interMovesOrder.begin(), interMovesOrder.end(), 1);
}

void LocalSearch::educate(Individual& indiv) {
    load(indiv);

    bool improved;
    int notImproved = 0;  // 0: intra   1: inter
    isIntraSearch = true;
    do {
        do {
            improved = (isIntraSearch) ? intraSearch() : interSearch();
            isIntraSearch = !isIntraSearch;
            notImproved = improved ? 0 : notImproved + 1;
        } while (notImproved < 2);

        improved = splitSearch(indiv);
    } while (improved);

    updateRoutesData();
    saveTo(indiv);
}

bool LocalSearch::splitSearch(Individual& indiv) {
    int prevTime = routes.back()->endTime;
    saveTo(indiv);
    split.split(&indiv);
    load(indiv);
    return (prevTime - indiv.eval) > 0;
}

/**************************************************************************
********************** INTRA SEARCH FUNCTIONS *****************************
***************************************************************************/

bool LocalSearch::intraSearch() {
    std::shuffle(intraMovesOrder.begin(), intraMovesOrder.end(), data.generator);
    bool improvedAny = false, improved;

    for (auto route : routes) {
        r1.route = route;
        r1.pos = route->pos;

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
    } else if (move == 6) {  // two opt
        return intraTwoOpt();
    }

    throw "Intra move id not known: " + move;
}

bool LocalSearch::intraSwap() {
    if (r1.route->nClients < b1Size + b2Size) return false;  // not enough clients to make blocks of these sizes

    bestImprovement = 0;
    intraSwapOneWay();
    if (b1Size != b2Size) {
        std::swap(b1Size, b2Size);
        intraSwapOneWay();
    }

    if (bestImprovement > 0) {  // found an improvement
        swapBlocks();
        return true;
    }
    return false;
}

void LocalSearch::intraSwapOneWay() {
    for (resetBlock1(); !blocks1Finished; moveBlock1Forward()) {
        preMinus = b1->prev->timeTo[b1->id]           // before b1
                   + b1End->timeTo[b1End->next->id];  // after b1

        for (resetBlock2Intra(); !blocks2Finished; moveBlock2Forward()) {
            minus = preMinus + b2End->timeTo[b2End->next->id];  // after b2
            plus = b1->prev->timeTo[b2->id]                     // new arc before b2
                   + b1End->timeTo[b2End->next->id];            // new arc after b1

            if (b1End->next == b2) {            // adjacent
                plus += b2End->timeTo[b1->id];  // after new b2 == before new b1
            } else {
                minus += b2->prev->timeTo[b2->id];         // old arc before b2
                plus += b2->prev->timeTo[b1->id]           // new arc before b1
                        + b2End->timeTo[b1End->next->id];  // new arc after b2
            }

            improvement = minus - plus;
            evaluateImprovement();
        }
    }
}

bool LocalSearch::intraRelocation() {
    bestImprovement = 0;
    for (resetBlock1(); !blocks1Finished; moveBlock1Forward()) {
        preMinus = b1->prev->timeTo[b1->id] + b1End->timeTo[b1End->next->id];
        prePlus = b1->prev->timeTo[b1End->next->id];

        for (b2 = &(r1.route->begin); b2->next != nullptr; b2 = b2->next) {
            if (b2->next == b1) {
                b2 = b1End->next;        // skip block
                if (b2->id == 0) break;  // no position after block
            }
            minus = preMinus + b2->timeTo[b2->next->id];
            plus = prePlus + b2->timeTo[b1->id] + b1End->timeTo[b2->next->id];

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

bool LocalSearch::intraTwoOpt() {
    bestImprovement = 0;
    for (b1 = r1.route->begin.next; b1->next != 0; b1 = b1->next) {
        minus = b1->prev->timeTo[b1->id] + b1->timeTo[b1->next->id];
        prePlus = 0;

        for (b1End = b1->next; b1End->id != 0; b1End = b1End->next) {
            minus += b1End->timeTo[b1End->next->id];
            prePlus += b1End->timeTo[b1End->prev->id];

            plus = prePlus + b1->prev->timeTo[b1End->id] + b1->timeTo[b1End->next->id];
            improvement = minus - plus;
            evaluateImprovement();
        }
    }

    if (bestImprovement > 0) {
        revertBlock();
        return true;
    }
    return false;
}

/**************************************************************************
********************** INTER SEARCH FUNCTIONS *****************************
***************************************************************************/

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

/**************************************************************************
************************** BLOCK FUNCTIONS ********************************
***************************************************************************/

void LocalSearch::resetBlock1() {
    blocks1Finished = false;

    b1 = r1.route->begin.next;
    b1End = b1;
    b1ReleaseDate = b1->releaseDate;
    b1Duration = 0;
    for (i = 1; i < b1Size; i++) {
        b1End = b1End->next;
        if (b1End->id == 0) {
            blocks1Finished = true;
            break;
        }
        b1ReleaseDate = std::max<int>(b1ReleaseDate, b1End->releaseDate);
        b1Duration += b1End->prev->timeTo[b1End->id];
    }
}

void LocalSearch::resetBlock2Intra() {
    blocks2Finished = false;

    b2 = b1End->next;
    b2End = b2;
    for (i = 1; i < b2Size; i++) {
        b2End = b2End->next;
    }

    if (b2End->id == 0) {
        // not possible position for block2 after block1 in that route
        blocks1Finished = true;
        blocks2Finished = true;
    }
}

void LocalSearch::resetBlock2Inter() {
    b2 = r2.route->begin.next;
    b2End = b2;
    b2ReleaseDate = b2->releaseDate;
    b2Duration = 0;
    for (i = 1; i < b2Size; i++) {
        b2End = b2End->next;
        b2ReleaseDate = std::max<int>(b2ReleaseDate, b2End->releaseDate);
        b2Duration += b2End->prev->timeTo[b2->id];
    }
    blocks2Finished = b2End->id == 0;
}

void LocalSearch::moveBlock1Forward() {
    b1 = b1->next;
    b1End = b1End->next;
    b1ReleaseDate = std::max<int>(b1->releaseDate, b1End->releaseDate);  // valid for blocks of size up to 2
    b1Duration = b1->durationAfter - b1End->durationAfter;
    if (b1End->id == 0) blocks1Finished = true;
}

void LocalSearch::moveBlock2Forward() {
    b2 = b2->next;
    b2End = b2End->next;
    b2ReleaseDate = std::max<int>(b2->releaseDate, b2End->releaseDate);  // valid for blocks of size up to 2
    b2Duration = b2->durationAfter - b2End->durationAfter;
    if (b2End->id == 0) blocks2Finished = true;
}

void LocalSearch::swapBlocks() {
#ifndef NDEBUG
    preMoveDebug("Swap");
#endif

    bestB1->prev->next = bestB2;
    aux = bestB1->prev;
    bestB1->prev = bestB2->prev;

    bestB2->prev->next = bestB1;
    bestB2->prev = aux;

    bestB1End->next->prev = bestB2End;
    aux = bestB1End->next;
    bestB1End->next = bestB2End->next;

    bestB2End->next->prev = bestB1End;
    bestB2End->next = aux;

#ifndef NDEBUG
    postMoveDebug("Swap");
#endif
}

void LocalSearch::relocateBlock() {
#ifndef NDEBUG
    bestB2End = bestB2->next;  // avoid seg fault
    preMoveDebug("Relocation");
#endif
    bestB1->prev->next = bestB1End->next;
    bestB1End->next->prev = bestB1->prev;

    aux = bestB2->next;
    bestB2->next->prev = bestB1End;
    bestB2->next = bestB1;

    bestB1->prev = bestB2;
    bestB1End->next = aux;
#ifndef NDEBUG
    postMoveDebug("Relocation");
#endif
}

void LocalSearch::revertBlock() {
#ifndef NDEBUG
    preMoveDebug("Revert");
#endif

    aux = bestB1End->next;
    for (node = bestB1; node != aux; node = node->prev) {
        std::swap(node->next, node->prev);
    }

    aux = bestB1End->prev;
    bestB1->next->next = bestB1End;
    bestB1End->prev = bestB1->next;

    bestB1->next = aux;
    aux->prev = bestB1;

#ifndef NDEBUG
    postMoveDebug("Revert");
#endif
}

/**************************************************************************
************************** OTHER FUNCTIONS ********************************
***************************************************************************/

bool LocalSearch::evaluateImprovement() {
    if (improvement > bestImprovement) {
        bestImprovement = improvement;
        bestB1 = b1;
        bestB1End = b1End;
        bestB2 = b2;
        bestB2End = b2End;
        return true;
    }
    return false;
}

bool LocalSearch::evaluateInterRouteImprovement() {
    if (r1.pos < r2.pos)
        routeA = &r1, routeB = &r2;
    else
        routeA = &r2, routeB = &r1;

    routesClearence = routeA->route->clearence[routeB->pos - 1];

    // evaluate a inter route search given that route1 and route2 new release dates are set by a inter route move
    // a inter route is evaluated w. r. t. the ending time of the second route in the move
    newRAEnd = std::max<int>(routeA->endBefore, routeA->newReleaseDate) + routeA->newDuration;
    deltaRAEnd = newRAEnd - routeA->route->endTime;

    newBeforeRBEnd = routeB->endBefore + std::min<int>(std::max<int>(0, deltaRAEnd - routesClearence),
                                                       std::max<int>(deltaRAEnd, routesClearence));
    newRBStart = std::max<int>(routeB->newReleaseDate, newBeforeRBEnd);
    newRBEnd = newRBStart + routeB->newDuration;

    improvement = routeB->route->endTime - newRBEnd;
    return evaluateImprovement();  //
}

void LocalSearch::updateRoutesData() {  // this data is only used during inter route moves
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

    // calculate the clearence between each pair of routes
    // first between adjacent routes
    int R = routes.size() - 1;
    for (int r = 0; r < R; r++) {
        routes[r]->clearence[r] = -INF;
        routes[r]->clearence[r + 1] = routes[r + 1]->releaseDate - routes[r]->endTime;
    }
    routes[R]->clearence[R] = -INF;

    int clearence, prevClearence;
    for (int r1 = 0; r1 < routes.size(); r1++) {
        clearence = routes[r1]->clearence[r1 + 1];
        for (int r2 = r1 + 2; r2 < routes.size(); r2++) {
            prevClearence = routes[r2 - 1]->clearence[r2];
            if (clearence > 0) {
                clearence += std::max<int>(prevClearence, 0);
            } else if (prevClearence > 0) {
                clearence = prevClearence;
            } else {
                clearence = std::max<int>(clearence, prevClearence);
            }
            routes[r1]->clearence[r2] = clearence;
        }
    }
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

void LocalSearch::checkEmptyRoute(Route* route) {
    if (route->begin.next->id == 0) {
        for (i = route->pos + 1; i < routes.size(); i++) routes[i]->pos--;
        routes.erase(routes.begin() + route->pos);
        emptyRoutes.push_back(route);
    }
}

void LocalSearch::load(const Individual& indiv) {
    // create the routes
    routes.clear();
    emptyRoutes.clear();
    addRoute();
    node = &(lastRoute->begin);
    lastRoute->nClients = 0;
    for (int i = 0; i < data.N; i++) {
        node->next = &(clients[indiv.giantTour[i]]);
        node->next->prev = node;
        node = node->next;
        lastRoute->nClients++;

        if (indiv.successors[indiv.giantTour[i]] == 0) {
            node->next = &(lastRoute->end);
            lastRoute->end.prev = node;

            if (i + 1 < data.N) {
                addRoute();
                node = &(lastRoute->begin);
                lastRoute->nClients = 0;
            }
        }
    }
    node->next = &(lastRoute->end);
    lastRoute->end.prev = node;
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

#ifndef NDEBUG

std::string LocalSearch::getRoutesStr() {  // for debugging
    std::string str = "    RD  |  DURAT |  START |   END  |  ROUTE\n";
    char buffer[100];
    int time;

    for (auto route : routes) {
        sprintf(buffer, "  %4d  |  %4d  |  %4d  |  %4d  |  ", route->releaseDate, route->duration, route->startTime,
                route->endTime);
        str += buffer;

        // print route
        str += "[ 0]";
        for (node = route->begin.next; node != nullptr; node = node->next) {
            time = node->prev->timeTo[node->id];
            sprintf(buffer, " -(%2d)-> [%2d]", time, node->id);
            str += buffer;
        }

        // print clearences
        str += "  Clearences: ";
        for (int r = route->pos + 1; r < routes.size(); r++) {
            sprintf(buffer, "(%d,%d)[%d]  ", route->pos, r, route->clearence[r]);
            str += buffer;
        }
        str += '\n';
    }
    return str;
}

std::string LocalSearch::getBlockStr(Node* bStart, Node* bEnd) {
    std::string blockStr = "[ ";
    for (; bStart != bEnd->next; bStart = bStart->next) {
        blockStr += std::to_string(bStart->id) + " ";
    }
    blockStr += "] ";
    return blockStr;
}

void LocalSearch::preMoveDebug(std::string move, bool bothBlocks) {
    updateRoutesData();
    _routeEnd = isIntraSearch || r1.pos > r2.pos ? r1.route->endTime : r2.route->endTime;

    std::string _moveType = (isIntraSearch ? "Intra" : "Inter");
    _log = "(" + _moveType + ") " + move + " " + getBlockStr(bestB1, bestB1End);
    if (bothBlocks) _log += " and " + getBlockStr(bestB2, bestB2End);
    _log += '\n' + getRoutesStr();
}

void LocalSearch::postMoveDebug(std::string move) {
    updateRoutesData();
    int _newEnd = isIntraSearch || r1.pos > r2.pos ? r1.route->endTime : r2.route->endTime;

    if (_newEnd >= _routeEnd) {
        std::cout << "A " << (isIntraSearch ? "Intra " : "Inter ") << move
                  << " was performed, but lead to no improvement: " << _routeEnd << " -> " << _newEnd << std::endl;
        std::cout << _log << "Result:" << std::endl << getRoutesStr() << std::endl;
    }
}

#endif
