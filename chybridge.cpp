#include "cvariant.h"
#include "chybridge.h"
#include "cchannel.h"
#include "ctransport.h"

#include <iostream>

#include <stdlib.h>

extern "C"
{

    HYBRIDGEC_EXPORT CHandlePtr hybridgeCreateChannel(CHandlePtr callback)
    {
        CChannel * c = new CChannel(callback);
        // std::cout << "hybridgeCreateChannel: " << callback << " -> " << c << std::endl;
        return c->stub();
    }

    HYBRIDGEC_EXPORT CHandlePtr hybridgeCreateTransport(CHandlePtr callback)
    {
        CTransport * t = new CTransport(callback);
        // std::cout << "hybridgeCreateChannel: " << callback << " -> " << c << std::endl;
        return t->stub();
    }

    HYBRIDGEC_EXPORT void *hybridgeAlloc(size_t size)
    {
        // std::cout << "hybridgeAlloc " << size << std::endl;
        return malloc(size);
    }

    HYBRIDGEC_EXPORT void hybridgeFree(void *ptr)
    {
        // std::cout << "hybridgeFree " << ptr << std::endl;
        free(ptr);
    }

    HYBRIDGEC_EXPORT void * hybridgeAllocBuffer(size_t size)
    {
        return CVariant::allocBuffer(size);
    }

    HYBRIDGEC_EXPORT void hybridgeFreeBuffer(size_t type, void * buffer)
    {
        CVariant::freeBuffer(static_cast<Value::Type>(type), buffer);
    }
}

