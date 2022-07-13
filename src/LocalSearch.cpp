#include "LocalSearch.h"

LocalSearch::LocalSearch(Data& data, Split& split)
    : data(data), split(split), clients(), routesObj(data.V + 1, {data}), routes(data.V + 1), intraMovesOrder(N_INTRA),
      interMovesOrder(N_INTER), routesOrder((data.V + 1) * data.V), nRoutesOrder(0) {
    routes.resize(0);  // resize but keep allocated space
    clients.reserve(data.V);
    for (int c = 0; c < data.V; c++) clients.emplace_back(data, c);

    std::iota(intraMovesOrder.begin(), intraMovesOrder.end(), 1);
    std::iota(interMovesOrder.begin(), interMovesOrder.end(), 1);
}

void LocalSearch::educate(Individual& indiv) {
    split.split(&indiv);
    load(indiv);
    updateRoutesData();

    bool improved;
    int notImproved = 0;
    moveType = 0;
    do {
        do {
            switch (moveType) {
                case 0:
                    improved = intraSearch();
                    break;
                case 1:
                    improved = interSearch();
                    break;
                case 2:
                    // improved = divideAndSwap();
                    break;
            }
            moveType = (moveType + 1) % 3;
            if (improved && moveType != 0) updateRoutesData();  // dont need to update for intra moves

            notImproved = improved ? 0 : notImproved + 1;
        } while (notImproved < 3);

        improved = splitSearch(indiv);
        updateRoutesData();
    } while (improved);

    saveTo(indiv);
}

bool LocalSearch::splitSearch(Individual& indiv) {
    int prevTime = routes.back()->endTime;
    saveTo(indiv);
    split.split(&indiv);
    load(indiv);
    return (prevTime - indiv.eval) > 0;
}

bool LocalSearch::divideAndSwap() {  // insert depot in a route, and swap the order
    return false;                    // todo remove
    for (r1.pos = 1; r1.pos < routes.size(); r1.pos++) {
        r1.route = routes[r1.pos];
        // if the ending time of the previous route is higher than the current route release date
        // it's not possible to improve the ending time of the current route by adding a depot
        // because the starting time of the newly generated route can't be less than the current route start time
        if (routes[r1.pos - 1]->endTime >= r1.route->releaseDate) continue;
        if (r1.route->end.prev->releaseDate == r1.route->releaseDate)
            continue;  // vertex with larger release date is at the end

        // skip the vertices before the one with higher release date
        b1 = r1.route->begin.next;
        while (b1->successorsRd == r1.route->releaseDate) b1 = b1->next;

        // from here, r1 = rb (first route when dividing, second route after swap) r2 = ra
        for (; b1->next->next->id != 0; b1 = b1->next) {
            newRAEnd = std::max<int>(b1->successorsRd, routes[r1.pos - 1]->endTime) +  // starting time
                       data.timesMatrix[0][b1->next->id] + b1->durationAfter;          // duration
            newRBEnd = std::max<int>(newRAEnd, b1->next->predecessorsRd) +             // starting time
                       b1->durationBefore + b1->timeTo[0];                             // duration

            if (newRBEnd < r1.route->endTime) {
                addRoute(r1.pos + 1);
                bestB1End = b1, bestB1 = r1.route->begin.next, bestB2 = &(lastRoute->begin);
                relocateBlock();
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
        b2Duration += b2End->prev->timeTo[b2End->id];
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
    bestB2End = (bestB2->next == nullptr) ? bestB2 : bestB2->next;  // avoid seg fault
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
    preMoveDebug("Revert", false);
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

// bool LocalSearch::evaluateInterRouteImprovement() {
//     if (r1.pos < r2.pos)
//         routeA = &r1, routeB = &r2;
//     else
//         routeA = &r2, routeB = &r1;

//     routesClearance = routeA->route->clearance[routeB->pos - 1];

//     // evaluate a inter route search given that route1 and route2 new release dates are set by a inter route move
//     // a inter route is evaluated w. r. t. the ending time of the second route in the move
//     newRAEnd = std::max<int>(routeA->endBefore, routeA->newReleaseDate) + routeA->newDuration;
//     deltaRAEnd = newRAEnd - routeA->route->endTime;

//     newBeforeRBEnd = routeB->endBefore + std::min<int>(std::max<int>(0, deltaRAEnd - routesClearance),
//                                                        std::max<int>(deltaRAEnd, routesClearance));
//     newRBStart = std::max<int>(routeB->newReleaseDate, newBeforeRBEnd);
//     newRBEnd = newRBStart + routeB->newDuration;

//     improvement = routeB->route->endTime - newRBEnd;
//     return evaluateImprovement();  //
// }

bool LocalSearch::evaluateInterRouteImprovement() {
    if (r1.pos < r2.pos)
        routeA = &r1, routeB = &r2;
    else
        routeA = &r2, routeB = &r1;

    routesClearance = routeA->route->clearance[routeB->pos - 1];

    // evaluate a inter route search given that route1 and route2 new release dates are set by a inter route move
    // a inter route is evaluated w. r. t. the ending time of the second route in the move
    deltaRAEnd =
        std::max<int>(routeA->endBefore, routeA->newReleaseDate) + routeA->newDuration - routeA->route->endTime;

    improvement = routeB->route->endTime -
                  (std::max<int>(routeB->newReleaseDate,
                                 routeB->endBefore + std::min<int>(std::max<int>(0, deltaRAEnd - routesClearance),
                                                                   std::max<int>(deltaRAEnd, routesClearance))) +
                   routeB->newDuration);
    return evaluateImprovement();  //
}

void LocalSearch::updateRoutesData() {  // this data is only used during inter route moves
    int prevEnd = 0;

    for (auto route : routes) {
        route->nClients = -1;

        // fill forward data
        for (node = route->begin.next; node != nullptr; node = node->next) {
            node->durationBefore = node->prev->durationBefore + node->prev->timeTo[node->id];
            node->predecessorsRd = std::max<int>(node->prev->predecessorsRd, node->prev->releaseDate);
            route->nClients++;
        }

        // fill backward data
        for (node = route->end.prev; node != nullptr; node = node->prev) {
            node->durationAfter = node->next->durationAfter + node->timeTo[node->next->id];
            node->successorsRd = std::max<int>(node->next->successorsRd, node->next->releaseDate);
        }

        route->releaseDate = route->end.predecessorsRd;
        route->duration = route->end.durationBefore;
        route->startTime = std::max<int>(route->releaseDate, prevEnd);
        route->endTime = route->startTime + route->duration;
        prevEnd = route->endTime;
    }

    // calculate the clearance between each pair of routes
    // first between adjacent routes
    int R = routes.size() - 1;
    for (int r = 0; r < R; r++) {
        routes[r]->clearance[r] = -INF;
        routes[r]->clearance[r + 1] = routes[r + 1]->releaseDate - routes[r]->endTime;
    }
    routes[R]->clearance[R] = -INF;

    int clearance, prevClearence;
    for (int r1 = 0; r1 < routes.size(); r1++) {
        clearance = routes[r1]->clearance[r1 + 1];
        for (int r2 = r1 + 2; r2 < routes.size(); r2++) {
            prevClearence = routes[r2 - 1]->clearance[r2];
            if (clearance > 0) {
                clearance += std::max<int>(prevClearence, 0);
            } else if (prevClearence > 0) {
                clearance = prevClearence;
            } else {
                clearance = std::max<int>(clearance, prevClearence);
            }
            routes[r1]->clearance[r2] = clearance;
        }
    }
}

void LocalSearch::addRoute() { addRoute(routes.size()); }

void LocalSearch::addRoute(int position) {
    if (emptyRoutes.empty()) {
        routes.insert(routes.begin() + position, &(routesObj[routes.size()]));
    } else {
        routes.insert(routes.begin() + position, emptyRoutes.back());
        emptyRoutes.pop_back();
    }
    lastRoute = routes[position];
    lastRoute->pos = position;
    lastRoute->begin.next = &(lastRoute)->end;
    lastRoute->end.prev = &(lastRoute)->begin;

    for (i = position + 1; i < routes.size(); i++) {
        routes[i]->pos = i;
    }
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
/**************************************************************************
************************** DEBUG FUNCTIONS ********************************
***************************************************************************/

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

        // print clearances
        str += "  Clearances: ";
        for (int r = route->pos + 1; r < routes.size(); r++) {
            sprintf(buffer, "(%d,%d)[%d]  ", route->pos, r, route->clearance[r]);
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

void LocalSearch::printRoutes() { std::cout << getRoutesStr() << std::endl; }

void LocalSearch::preMoveDebug(std::string move, bool bothBlocks) {
    checkRoutesData();
    updateRoutesData();
    _routeEnd = r1.route->endTime;
    if (moveType == 1 && r2.pos > r1.pos) _routeEnd = r2.route->endTime;

    std::string _moveType = (moveType == 0 ? "Intra" : "Inter");
    _log = "(" + _moveType + ") " + move + " " + getBlockStr(bestB1, bestB1End);
    if (bothBlocks) _log += " and " + getBlockStr(bestB2, bestB2End);
    _log += '\n' + getRoutesStr();
}

void LocalSearch::postMoveDebug(std::string move) {
    updateRoutesData();
    int _newEnd;
    if (moveType == 0 || r1.pos > r2.pos)
        _newEnd = r1.route->endTime;
    else if (moveType == 1)
        _newEnd = r2.route->endTime;
    else if (moveType == 2)
        _newEnd = lastRoute->endTime;

    if (_newEnd >= _routeEnd) {
        std::cout << "A " << (moveType == 0 ? "Intra " : "Inter ") << move
                  << " was performed, but lead to no improvement: " << _routeEnd << " -> " << _newEnd << std::endl;
        std::cout << _log << "Result:" << std::endl << getRoutesStr() << std::endl;
    }
}

#include <assert.h>

void LocalSearch::checkRoutesData() {
    int endTime = 0;
    for (auto route : routes) {
        int releaseDate = 0, duration = 0, nClients = -1;

        for (auto node = route->begin.next; node != nullptr; node = node->next) {
            assert(node->prev->next == node);
            assert(node->releaseDate == data.releaseDates[node->id]);
            assert(node->predecessorsRd == releaseDate);
            releaseDate = std::max<int>(releaseDate, node->releaseDate);
            duration += node->prev->timeTo[node->id];
            assert(node->durationBefore == duration);
            nClients++;
        }

        assert(route->nClients == nClients);
        assert(route->releaseDate == releaseDate);
        assert(route->startTime == std::max<int>(endTime, releaseDate));
        assert(route->duration == duration);
        endTime = std::max<int>(endTime, releaseDate) + duration;
        assert(route->endTime == endTime);

        releaseDate = 0, duration = 0;
        for (auto node = route->end.prev; node != nullptr; node = node->prev) {
            assert(node->next->prev == node);
            assert(node->successorsRd == releaseDate);
            releaseDate = std::max<int>(releaseDate, data.releaseDates[node->id]);
            duration += node->timeTo[node->next->id];
            assert(node->durationAfter == duration);
        }
    }
}

#endif
