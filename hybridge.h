#ifndef HYBRIDGE_H
#define HYBRIDGE_H

#include "HybridgeC_global.h"

extern "C"
{
    HYBRIDGEC_EXPORT extern void * hybridgeAlloc(size_t size);
    HYBRIDGEC_EXPORT extern void hybridgeFree(void * ptr);
    HYBRIDGEC_EXPORT extern void * allocBuffer(size_t size);
    HYBRIDGEC_EXPORT extern void resetBuffer();
}

#endif // HYBRIDGE_H
