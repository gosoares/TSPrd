#include "LocalSearch.h"

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
