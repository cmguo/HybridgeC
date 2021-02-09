#ifndef CHANDLE_H
#define CHANDLE_H

#ifdef __cplusplus

template <typename C>
struct CHandle
{
    C * callback;
};

typedef CHandle<void>* CHandlePtr;
typedef CHandle<void> const * CConstHandlePtr;

template <typename T, typename F>
inline CHandle<T> * cast(CHandle<F> * handle)
{
    return reinterpret_cast<CHandle<T>*>(handle);
}

template <typename T, typename F>
inline CHandle<T> const * cast(CHandle<F> const * handle)
{
    return reinterpret_cast<CHandle<T>*>(handle);
}

#else

typedef struct CHandle
{
    void * callback;
} * CHandlePtr;

typedef struct CHandle const * CConstHandlePtr;

#endif // __cplusplus

#endif // CHANDLE_H
