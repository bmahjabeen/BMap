#ifndef BMAP_CONSTANTS_H
#define BMAP_CONSTANTS_H

static const int REQUEST_BUFFER_SZ = 1024*2;
static const int RESPONSE_BUFFER_SZ = 1024*10;
static const int MIN_PORT = 10000;

enum Command {
    Exit,
    AddVertex,
    AddEdge,
    PrintMap,
    GetTrip,
    FindPoi,
    AddEdgeEvent,
    Store,
    Retrieve,
    Reset,
    Invalid
};

#endif //BMAP_CONSTANTS_H
