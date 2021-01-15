#ifndef CVARIANT_H
#define CVARIANT_H

#include <core/value.h>

class CVariant
{
public:
    static void fromValue(Value const & from, void * to);

    static void toValue(Value & to, void * from);

    static void * fromValue(Value const & from, bool copy = false);

    static Value toValue(Value::Type type, void * from, bool copy = false);

    static char * allocBuffer(size_t size);

    static void resetBuffer();
};

#endif // CVARIANT_H
