#ifndef CHYBRIDGE_H
#define CHYBRIDGE_H

#include "HybridgeC_global.h"
#include "chandle.h"

extern "C"
{

    HYBRIDGEC_EXPORT extern CHandlePtr hybridgeCreateTransport(CHandlePtr callback);

    HYBRIDGEC_EXPORT extern void * hybridgeAlloc(size_t size);
    HYBRIDGEC_EXPORT extern void hybridgeFree(void * ptr);

    HYBRIDGEC_EXPORT extern void * allocBuffer(size_t size);
    HYBRIDGEC_EXPORT extern void resetBuffer();
}

#endif // CHYBRIDGE_H
