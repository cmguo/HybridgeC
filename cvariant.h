#ifndef CVARIANT_H
#define CVARIANT_H

#ifdef __cplusplus

#include <core/value.h>

class CVariant
{
public:
    static void * allocBuffer(size_t size);

    static void freeBuffer(Value::Type type, void * buffer);

public:
    CVariant(Value::Type type = Value::None, void * buffer = nullptr);

    CVariant(Value const & from, bool copy = false);

    CVariant(CVariant && o);

    DELETE_COPY(CVariant)

    ~CVariant();

    CVariant & operator=(CVariant && o);

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

#endif

void * cVariantEncodeString(char const * string);
char const * cVariantDecodeString(void * variant);

struct CArrayEntry
{
    Value::Type type;
    void * value;
};

typedef void (*ArrayVisitor)(void * array, size_t index, CArrayEntry * entry);

void * cVariantEncodeArray(void * array, size_t count, ArrayVisitor visitor);
void * cVariantDecodeArray(void * array, void * outArr, ArrayVisitor visitor);

struct CMapEntry
{
    char * key;
    Value::Type type;
    void * value;
};

typedef void (*MapVisitor)(void * map, size_t index, CMapEntry * entry);

void * cVariantEncodeMap(void * map, size_t count, MapVisitor visitor);
void * cVariantDecodeMap(void * map, void * outMap, MapVisitor visitor);

#endif // CVARIANT_H
