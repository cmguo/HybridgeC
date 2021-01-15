#include "cvariant.h"

template<typename T>
static size_t copy(void const * f, void * t)
{
    if (t)
        new (t) T(*reinterpret_cast<T const *>(f));
    return sizeof (T);
}

template<typename T>
struct Copy
{
    static constexpr char const * magic = nullptr;
};

template<> struct Copy<std::string>
{
    static constexpr char const * magic = "STR";
    static void copyFrom(std::string const & f, void * t, size_t & n)
    {
        n = f.length() + 1;
        if (t)
            memcpy(t, f.c_str(), n + 1);
    }
    static void copyTo(std::string & t, void * f, size_t n)
    {
        t.assign(reinterpret_cast<char *>(f), n);
    }
};

template<> struct Copy<Array>
{
    static constexpr char const * magic = "ARR";
    struct Entry
    {
        size_t type;
        void * value;
    };
    static void copyFrom(Array const & f, void * t, size_t & n)
    {
        n = f.size();
        if (t) {
            Entry * entries = reinterpret_cast<Entry*>(t);
            for (size_t i = 0; i < n; ++i) {
                entries[i].type = static_cast<size_t>(f.at(i).type());
                CVariant::fromValue(f.at(i), &entries[i].value);
            }
        }
        n *= sizeof (Entry);
    }
    static void copyTo(Array & t, void const * f, size_t n)
    {
        n /= sizeof (Entry);
        Entry const * entries = reinterpret_cast<Entry const *>(f);
        for (size_t i = 0; i < n; ++i) {
            t.push_back(CVariant::toValue(static_cast<Value::Type>(entries[i].type), entries[i].value));
        }
    }
};

template<> struct Copy<Map>
{
    static constexpr char const * magic = "MAP";
    struct Entry
    {
        void * key;
        size_t type;
        void * value;
    };
    static void copyFrom(Map const & f, void * t, size_t & n)
    {
        n = f.size();
        if (t) {
            Entry * entries = reinterpret_cast<Entry*>(t);
            auto it = f.begin();
            for (size_t i = 0; i < n; ++i) {
                entries[i].key = CVariant::fromValue(it->first);
                entries[i].type = static_cast<size_t>(it->second.type());
                entries[i].value = CVariant::fromValue(it->second);
            }
        }
        n *= sizeof (Entry);
    }
    static void copyTo(Map & t, void const * f, size_t n)
    {
        n /= sizeof (Entry);
        Entry const * entries = reinterpret_cast<Entry const *>(f);
        for (size_t i = 0; i < n; ++i) {
            t.emplace(CVariant::toValue(Value::String, entries[i].key).toString(),
                        CVariant::toValue(static_cast<Value::Type>(entries[i].type), entries[i].value));
        }
    }
};

template<typename T>
static size_t copy2(void const * f, void * t)
{
    if (memcmp(f, Copy<T>::magic, 4) == 0) {
        if (t) {
            T * tt = new (t) T();
            size_t n = static_cast<size_t const *>(f)[1];
            void * ff = static_cast<void * const *>(f)[2];
            Copy<T>::copyTo(*tt, ff, n);
        }
        return sizeof (T);
    } else {
        if (t) {
            T const * ff = reinterpret_cast<T const *>(f);
            size_t n = 0;
            Copy<T>::copyFrom(*ff, nullptr, n);
            memcpy(t, Copy<T>::magic, 4);
            reinterpret_cast<size_t*>(t)[1] = n;
            void * tt = CVariant::allocBuffer(n);
            reinterpret_cast<void**>(t)[2] = tt;
            Copy<T>::copyFrom(*ff, tt, n);
        }
        return 16;
    }
}

static size_t copyObject(void const * f, void * t)
{
    return copy<Object*>(f, t);
}

static constexpr size_t (*copies[])(void const *, void *) = {
        &copy<bool>, &copy<int>, &copy<long long>,
        &copy<float>, &copy<double>, &copy2<std::string>,
        &copy2<Array>, &copy2<Map>, &copyObject,
};

void CVariant::fromValue(Value const & from, void * to)
{
    copies[from.type()](from.value(), to);
}

void CVariant::toValue(Value & to, void * from)
{
    copies[to.type()](from, to.value());
}

void * CVariant::fromValue(Value const & from, bool copy)
{
    if (!copy && from.type() < Value::String)
        return const_cast<void*>(from.value());
    size_t sz = copies[from.type()](from.value(), nullptr);
    void * to = allocBuffer(sz);
    copies[from.type()](from.value(), to);
    return to;
}

Value CVariant::toValue(Value::Type type, void *from, bool copy)
{
    Value to(type, from);
    if (copy || type >= Value::String) {
        size_t sz = copies[to.type()](from, nullptr);
        to = Value::adopt(type, new char[sz]);
        copies[to.type()](from, to.value());
    }
    return to;
}

static std::vector<char*> buffers;

char * CVariant::allocBuffer(size_t size)
{
    char * b = new char[size];
    buffers.push_back(b);
    return b;
}

void CVariant::resetBuffer()
{
    for (auto b : buffers)
        delete [] b;
    buffers.clear();
}
