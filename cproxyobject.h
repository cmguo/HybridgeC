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

    CProxyObject(HandlePtr channel);

    ~CProxyObject() override;

    virtual void * handle() const override { return handle_; }

public:
    static CProxyObject * fromCallback(HandlePtr callback);

    bool getProperty(char const * property, void * value);

    bool setProperty(char const * property, void * value);

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
    const char *(*metaData)(HandlePtr handle);
    size_t (*readProperty)(HandlePtr handle, const Object *object, char const * property, void * result);
    size_t (*writeProperty)(HandlePtr handle, Object *object, char const * property, void * value);
    size_t (*invokeMethod)(HandlePtr handle, Object *object, char const * method, void ** args, HandlePtr response);
};


#endif // CPROXYOBJECT_H
