#ifndef CPROXYOBJECT_H
#define CPROXYOBJECT_H

#include "chandle.h"

#include <core/proxyobject.h>

struct ProxyObjectStub;

class CProxyObject : public ProxyObject
{
public:
    struct ResultCallback
    {
        size_t (*apply)(CHandlePtr handle, void * result);
    };

    struct SignalCallback
    {
        size_t (*apply)(CHandlePtr handle, Object const * object, size_t signalIndex, void ** args);
    };

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
    CHandle<ProxyObjectStub> callback_;
    CHandlePtr handle_;
};

struct ProxyObjectStub
{
    static ProxyObjectStub instance;
    const char *(*metaData)(CHandlePtr object);
    void * (*readProperty)(CHandlePtr object, char const * property);
    size_t (*writeProperty)(CHandlePtr object, char const * property, void * value);
    size_t (*invokeMethod)(CHandlePtr object, char const * method, void ** args, CHandlePtr response);
    size_t (*connect)(CHandlePtr object, size_t signalIndex, CHandlePtr handler);
    size_t (*disconnect)(CHandlePtr object, size_t signalIndex, CHandlePtr handler);
};


#endif // CPROXYOBJECT_H
