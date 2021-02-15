#ifndef CPROXYOBJECT_H
#define CPROXYOBJECT_H

#include "chandle.h"

#include <stdlib.h>

struct CProxyObjectResultCallback
{
    void (*apply)(CHandlePtr handle, void * result);
};

struct CProxyObjectSignalCallback
{
    void (*apply)(CHandlePtr handle, CConstHandlePtr object, size_t signalIndex, void ** args);
};

#ifdef __cplusplus

#include <core/proxyobject.h>

struct CProxyObjectStub;

class CProxyObject : public ProxyObject
{
public:
    typedef CProxyObjectResultCallback ResultCallback;

    typedef CProxyObjectSignalCallback SignalCallback;

    CProxyObject(CHandlePtr channel, Map &&classinfo);

    ~CProxyObject() override;

    virtual void * handle() const override { return handle_; }

public:
    static CProxyObject * fromCallback(CHandlePtr callback);

    char const * metaData();

    void * readProperty(char const * property);

    bool writeProperty(char const * property, void * value);

    bool invokeMethod(char const * method, void ** args, CHandlePtr onResult);

    bool connect(size_t signalIndex, CHandlePtr handler);

    bool disconnect(size_t signalIndex, CHandlePtr handler);

private:
    CHandle<CProxyObjectStub> callback_;
    CHandlePtr handle_;
};

#endif

struct CProxyObjectStub
{
    const char * (*metaData)(CHandlePtr object);
    void * (*readProperty)(CHandlePtr object, char const * property);
    size_t (*writeProperty)(CHandlePtr object, char const * property, void * value);
    size_t (*invokeMethod)(CHandlePtr object, char const * method, void ** args, CHandlePtr response);
    size_t (*connect)(CHandlePtr object, size_t signalIndex, CHandlePtr handler);
    size_t (*disconnect)(CHandlePtr object, size_t signalIndex, CHandlePtr handler);
};

#endif // CPROXYOBJECT_H
