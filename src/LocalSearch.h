#ifndef TSPRD_LOCALSEARCH_H
#define TSPRD_LOCALSEARCH_H

#include "Data.h"
#include "Population.h"

#define N_INTRA 6  // number of implemented intra moves
#define N_INTER 0  // number of implemented inter moves

struct Node {
    const int id;                    // id of the client, 0 represents the depot
    const int releaseDate;           // release date of that client
    const std::vector<int>& timeTo;  // time from this node to all others (from times matrix)

    int durationBefore;  // sum of travel times of arcs after this client
    int durationAfter;   // sum of travel times of arcs before this client
    int predecessorsRd;  // the bigger release date of clients before this
    int successorsRd;    // the bigger release date of clients after this

    Node* next;
    Node* prev;

    Node(Data& data, int id)
        : id(id), releaseDate(data.releaseDates[id]), timeTo(data.timesMatrix[id]), durationBefore(0), durationAfter(0),
          predecessorsRd(0), successorsRd(0), next(nullptr), prev(nullptr) {}
};

struct Route {
    int pos;  // the position of the route in the solution

    // two nodes that represents the depot at the begin and end of the routes
    Node begin;
    Node end;

    int nClients;     // how many clients in that route
    int releaseDate;  // max of clients release dates
    int startTime;    // route starting time
    int duration;     // total travel time in the route
    int endTime;      // startingTime + duration

    Route(Data& data) : begin(data, 0), end(data, 0) {}
};

class LocalSearch {
   private:
    Data& data;
    Split& split;
    std::vector<Node> clients;  // nodes for all clients (0 is undefined)

    std::vector<Route> routesObj;     // objects for routes
    std::vector<Route*> routes;       // routes of the solution, in order
    std::vector<Route*> emptyRoutes;  // routes that became empty and were removed from `routes` and may get reused
    std::vector<std::vector<int> > routesCoupling;

    std::vector<int> intraMovesOrder;  // order to apply the intra moves
    std::vector<int> interMovesOrder;  // order to apply the intra moves

    // Auxiliary variables used within the local search functions
    Route *route1, *route2, *lastRoute;
    Node *node, *aux;
    int i, whichMove, move, pos, preMinus, prePlus, minus, plus;
    int improvement, bestImprovement;

    // variables representing two blocks on which moves are made
    Node *b1, *b1End, *b2, *b2End;
    Node *bestB1, *bestB1End, *bestB2, *bestB2End;
    int b1Size, b2Size;
    bool blocksFinished;

   public:
    LocalSearch(Data& data, Split& split);

    void educate(Individual& indiv);
    bool splitSearch(Individual& indiv);

    bool intraSearch();
    bool callIntraSearch();
    bool intraSwap();
    void intraSwapOneWay();
    bool intraRelocation();
    bool intraTwoOpt();

    bool interSearch();
    bool callInterSearch();

    // functions that operates in blocks b1 and b2
    void resetBlock1();        // set block1 to start of route1
    void resetBlock2Intra();   // set block2 do immediatly after block1
    void resetBlock2Inter();   // set block2 to start of route 2
    void moveBlock1Forward();  // move block1 to next position in its route
    void moveBlock2Forward();  // move block2 to next position in its route
    void swapBlocks();         // swap block1 and block2
    void relocateBlock();      // relocate block1 to after block2Start
    void revertBlock();        // revert block1

    void evaluateImprovement();  // if improved, save current blocks in best blocks pointers
    void updateRoutesData();
    void addRoute();  // add a route to the end of routes

    void load(const Individual& indiv);
    void saveTo(Individual& indiv);
};

#endif  // TSPRD_LOCALSEARCH_H
