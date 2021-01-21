#ifndef CVARIANT_H
#define CVARIANT_H

#include <core/value.h>

class CVariant
{
public:
    static void * allocBuffer(size_t size);

    static void freeBuffer(Value::Type type, void * buffer);

public:
    CVariant(Value::Type type = Value::None, void * buffer = nullptr);

    CVariant(Value const & from, bool copy = false);

    ~CVariant();

    operator void *() const
    {
        return buffer_;
    }

    void * detach();

    Value toValue(bool copy = false);

private:
    Value::Type type_;
    void * buffer_;
};

class CVariantArgs
{
public:
    CVariantArgs(Array &&args);

    operator void **();

private:
    std::vector<CVariant> vars_;
    std::vector<void*> args_;
};

#endif // CVARIANT_H
