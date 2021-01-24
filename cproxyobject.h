#ifndef CPROXYOBJECT_H
#define CPROXYOBJECT_H

#include "handleptr.h"

#include <core/proxyobject.h>

struct ProxyObjectStub;

class CProxyObject : public ProxyObject
{
public:
    struct ResultCallback
    {
        size_t (*apply)(HandlePtr handle, void * result);
    };

    struct SignalCallback
    {
        size_t (*apply)(HandlePtr handle, Object const * object, size_t signalIndex, void ** args);
    };

    CProxyObject(HandlePtr channel, Map &&classinfo);

    ~CProxyObject() override;

    virtual void * handle() const override { return handle_; }

public:
    static CProxyObject * fromCallback(HandlePtr callback);

    char const * metaData();

    void * readProperty(char const * property);

    bool writeProperty(char const * property, void * value);

    bool invokeMethod(char const * method, void ** args, HandlePtr onResult);

    bool connect(size_t signalIndex, HandlePtr handler);

    bool disconnect(size_t signalIndex, HandlePtr handler);

private:
    Handle<ProxyObjectStub> callback_;
    HandlePtr handle_;
};

struct ProxyObjectStub
{
    static ProxyObjectStub instance;
    const char *(*metaData)(HandlePtr object);
    void * (*readProperty)(HandlePtr object, char const * property);
    size_t (*writeProperty)(HandlePtr object, char const * property, void * value);
    size_t (*invokeMethod)(HandlePtr object, char const * method, void ** args, HandlePtr response);
    size_t (*connect)(HandlePtr object, size_t signalIndex, HandlePtr handler);
    size_t (*disconnect)(HandlePtr object, size_t signalIndex, HandlePtr handler);
};


#endif // CPROXYOBJECT_H
