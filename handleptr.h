#ifndef HANDLEPTR_H
#define HANDLEPTR_H

template <typename C>
struct Handle
{
    C * callback;
};

typedef Handle<void>* HandlePtr;

template <typename T, typename F>
inline Handle<T> * cast(Handle<F> * handle)
{
    return reinterpret_cast<Handle<T>*>(handle);
}

#endif // HANDLEPTR_H
