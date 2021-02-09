#ifndef CCHANNEL_H
#define CCHANNEL_H

#include "HybridgeC_global.h"
#include "cmetaobject.h"

#include <core/channel.h>

#include <map>

class CChannel : public Channel
{
public:
    struct Callback
    {
        CHandlePtr (*metaObject)(void const * handle, const Object *object);
        char const * (*createUuid)(void const * handle);
        CHandlePtr (*createProxyObject)(void const * handle, CHandlePtr object);
        void (*startTimer)(void * handle, int msec);
        void (*stopTimer)(void * handle);
    };

    CChannel(CHandlePtr handle);

    using Channel::timerEvent;

    // Channel interface
protected:
    virtual MetaObject *metaObject(const Object *object) const override;
    virtual std::string createUuid() const override;
    virtual ProxyObject *createProxyObject(Map &&classinfo) const override;
    virtual void startTimer(int msec) override;
    virtual void stopTimer() override;

private:
    CHandle<Callback> * handle_;
    mutable std::map<CHandlePtr, CMetaObject*> metaobjs_;
};

struct ChannelStub
{
    void * (*create)(CHandlePtr handle);
    void (*registerObject)(void * channel, char const * name, void * object);
    void (*deregisterObject)(void * channel, void * object);
    bool (*blockUpdates)(void * channel);
    void (*setBlockUpdates)(void * channel, bool block);
    void (*connectTo)(void * channel, void * transport, CHandlePtr response);
    void (*disconnectFrom)(void * channel, void * transport);
    void (*timerEvent)(void * channel);
    void (*free)(void * channel);
};

extern "C"
{
HYBRIDGEC_EXPORT extern struct ChannelStub channelStub;
}

#endif // CCHANNEL_H
