#ifndef CCHANNEL_H
#define CCHANNEL_H

#include "HybridgeC_global.h"
#include "cmetaobject.h"

struct CChannelCallback
{
    CHandlePtr (*metaObject)(CHandlePtr handle, const void *object);
    char const * (*createUuid)(CHandlePtr handle);
    CHandlePtr (*createProxyObject)(CHandlePtr handle, CHandlePtr object);
    void (*startTimer)(CHandlePtr handle, int msec);
    void (*stopTimer)(CHandlePtr handle);
};

#ifdef __cplusplus

#include <core/channel.h>

#include <map>

struct CChannelStub;

class CChannel : public Channel
{
public:
    typedef CChannelCallback Callback;

    CChannel(CHandlePtr handle);

    using Channel::timerEvent;

public:
    CHandlePtr stub();
    
    static CChannel * fromCallback(CHandlePtr callback);

    // Channel interface
protected:
    virtual MetaObject *metaObject(const Object *object) const override;
    virtual std::string createUuid() const override;
    virtual ProxyObject *createProxyObject(Map &&classinfo) const override;
    virtual void startTimer(int msec) override;
    virtual void stopTimer() override;

    CMetaObject *metaObject(CHandle<CMetaObject::Callback> * handle) const;

private:
    CHandle<CChannelStub> stub_;
    CHandle<Callback> * handle_;
    mutable std::map<CHandle<CMetaObject::Callback> *, CMetaObject*> metaobjs_;
};

#endif

struct CChannelStub
{
    void (*registerObject)(CHandlePtr channel, char const * name, void * object);
    void (*deregisterObject)(CHandlePtr channel, void * object);
    bool (*blockUpdates)(CHandlePtr channel);
    void (*setBlockUpdates)(CHandlePtr channel, bool block);
    void (*connectTo)(CHandlePtr channel, void * transport, CHandlePtr response);
    void (*disconnectFrom)(CHandlePtr channel, void * transport);
    void (*timerEvent)(CHandlePtr channel);
    void (*free)(CHandlePtr channel);
};

#endif // CCHANNEL_H
