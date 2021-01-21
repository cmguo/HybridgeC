#include "cvariant.h"
#include "hybridge.h"

#include <iostream>

#include <stdlib.h>

extern "C"
{

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

    HYBRIDGEC_EXPORT void *allocBuffer(size_t size)
    {
        return CVariant::allocBuffer(size);
    }

    HYBRIDGEC_EXPORT void freeBuffer(size_t type, void * buffer)
    {
        CVariant::freeBuffer(static_cast<Value::Type>(type), buffer);
    }
}

