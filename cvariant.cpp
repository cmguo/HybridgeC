#include "cvariant.h"

#include <iostream>
#include <list>

template<typename T>
static size_t copy(void const * f, void * t, bool)
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

struct Header
{
    union {
        char magic[4];
        size_t magic2;
    };
    size_t size;
    void * block;
    void alloc(size_t n) {
        size = n;
        block = CVariant::allocBuffer(n);
    }
};

template<> struct Copy<std::string>
{
    static constexpr char const * magic = "STR";
    static void copyFrom(std::string const & f, Header & h, bool copy)
    {
        if (copy) {
            h.alloc(f.length() + 1);
            memcpy(h.block, f.c_str(), h.size);
        } else {
            h.size = f.length() + 1;
            h.block = const_cast<char *>(f.c_str());
        }
    }
    static void copyTo(std::string & t, Header const & h, bool)
    {
        t.assign(reinterpret_cast<char *>(h.block), h.size - 1);
    }
    static void free(Header &) {
    }
};

template<> struct Copy<Array>
{
    static constexpr char const * magic = "ARR";
    struct Entry
    {
        Value::Type type;
        void * value;
    };
    static void copyFrom(Array const & f, Header & h, bool copy)
    {
        h.alloc(f.size() * sizeof (Entry));
        Entry * entries = reinterpret_cast<Entry*>(h.block);
        for (size_t i = 0; i < f.size(); ++i) {
            entries[i].type = f.at(i).type();
            entries[i].value = CVariant(f.at(i), copy).detach();
        }
    }
    static void copyTo(Array & t, Header const & h, bool copy)
    {
        size_t n = h.size / sizeof (Entry);
        Entry const * entries = reinterpret_cast<Entry const *>(h.block);
        for (size_t i = 0; i < n; ++i) {
            t.emplace_back(CVariant(entries[i].type, entries[i].value).toValue(copy));
        }
    }
    static void free(Header & h) {
        size_t n = h.size / sizeof (Entry);
        Entry * entries = reinterpret_cast<Entry *>(h.block);
        for (size_t i = 0; i < n; ++i) {
            CVariant::freeBuffer(entries->type, entries->value);
        }
    }
};

template<> struct Copy<Map>
{
    static constexpr char const * magic = "MAP";
    struct Entry
    {
        void * key;
        Value::Type type;
        void * value;
    };
    static void copyFrom(Map const & f, Header & h, bool copy)
    {
        h.alloc(f.size() * sizeof (Entry));
        Entry * entries = reinterpret_cast<Entry*>(h.block);
        auto it = f.begin();
        for (size_t i = 0; i < f.size(); ++i, ++it) {
            Header h;
            Copy<std::string>::copyFrom(it->first, h, copy);
            entries[i].key = h.block;
            entries[i].type = it->second.type();
            entries[i].value = CVariant(it->second, copy).detach();
        }
    }
    static void copyTo(Map & t, Header const & h, bool copy)
    {
        size_t n = h.size / sizeof (Entry);
        Entry const * entries = reinterpret_cast<Entry const *>(h.block);
        for (size_t i = 0; i < n; ++i) {
            t.emplace(static_cast<char const *>(entries[i].key),
                        CVariant(entries[i].type, entries[i].value).toValue(copy));
        }
    }
    static void free(Header & h) {
        size_t n = h.size / sizeof (Entry);
        Entry * entries = reinterpret_cast<Entry *>(h.block);
        for (size_t i = 0; i < n; ++i) {
            CVariant::freeBuffer(Value::None, entries->key);
            CVariant::freeBuffer(entries->type, entries->value);
        }
    }
};

template<typename T>
static size_t copy2(void const * f, void * t, bool copy)
{
    if (f == nullptr) {
        assert(memcmp(t, Copy<T>::magic, 4) == 0);
        Header & h = *reinterpret_cast<Header *>(t);
        Copy<T>::free(h);
        CVariant::freeBuffer(Value::None, h.block);
        return 0;
    }
    if (memcmp(f, Copy<T>::magic, 4) == 0) {
        if (t) {
            T * tt = new (t) T();
            Copy<T>::copyTo(*tt, *reinterpret_cast<Header const *>(f), copy);
        }
        return sizeof (T);
    } else {
        if (t) {
            T const * ff = reinterpret_cast<T const *>(f);
            memcpy(t, Copy<T>::magic, 4);
            Copy<T>::copyFrom(*ff, *reinterpret_cast<Header *>(t), copy);
        }
        return sizeof (Header);
    }
}

static size_t copyObject(void const * f, void * t, bool c)
{
    return copy<Object*>(f, t, c);
}

static constexpr size_t (*copies[])(void const *, void *, bool) = {
        nullptr,
        &copy<bool>, &copy<int>, &copy<long long>,
        &copy<float>, &copy<double>, &copy2<std::string>,
        &copy2<Array>, &copy2<Map>, &copyObject,
};

static std::list<void*> buffers;

void * CVariant::allocBuffer(size_t size)
{
    void * buffer = new char[size];
    std::cout << "allocBuffer " << buffer << std::endl;
    buffers.push_back(buffer);
    return buffer;
}

void CVariant::freeBuffer(Value::Type type, void *buffer)
{
    auto it = std::find(buffers.begin(), buffers.end(), buffer);
    if (it == buffers.end())
        return;
    if (type >= Value::String && type < Value::Object_) {
        copies[type](nullptr, buffer, false);
    }
    std::cout << "freeBuffer " << buffer << std::endl;
    delete [] reinterpret_cast<char*>(buffer);
    buffers.erase(it);
}

CVariant::CVariant(Value::Type type, void *buffer)
    : type_(type)
    , buffer_(buffer)
{
}

CVariant::CVariant(Value const & from, bool copy)
    : type_(from.type())
    , buffer_(const_cast<void*>(from.value()))
{
    if (!copy && from.type() < Value::String)
        return;
    size_t sz = copies[from.type()](from.value(), nullptr, copy);
    buffer_ = allocBuffer(sz);
    copies[from.type()](from.value(), buffer_, copy);
}

CVariant::CVariant(CVariant &&o)
    : type_(o.type_)
    , buffer_(o.buffer_)
{
    o.buffer_ = nullptr;
}

CVariant::~CVariant()
{
    if (buffer_)
        freeBuffer(type_, buffer_);
}

CVariant &CVariant::operator=(CVariant &&o)
{
    CVariant t(std::move(o));
    std::swap(type_, t.type_);
    std::swap(buffer_, t.buffer_);
    return *this;
}

void *CVariant::detach()
{
    void * buffer = buffer_;
    buffer_ = nullptr;
    return buffer;
}

Value CVariant::toValue(bool copy)
{
    Value to(type_, buffer_);
    if (copy || type_ >= Value::String) {
        size_t sz = copies[to.type()](buffer_, nullptr, copy);
        to = Value::adopt(type_, new char[sz]);
        copies[to.type()](buffer_, to.value(), copy);
    }
    return to;
}

CVariantArgs::CVariantArgs(Array &&args)
    : vars_(args.size())
    , args_(args.size())
{
    for (size_t i = 0; i < args.size(); ++i) {
        vars_[i] = CVariant(args[i]);
        args_[i] = vars_[i];
    }
}

CVariantArgs::operator void **()
{
    return args_.empty() ? nullptr : &args_[0];
}
