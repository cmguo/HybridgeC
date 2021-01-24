#ifndef CCHANNEL_H
#define CCHANNEL_H

#include "HybridgeC_global.h"
#include "cmeta.h"

#include <core/channel.h>

#include <map>

class CChannel : public Channel
{
public:
    struct Callback
    {
        HandlePtr (*metaObject)(void const * handle, const Object *object);
        char const * (*createUuid)(void const * handle);
        HandlePtr (*createProxyObject)(void const * handle, HandlePtr object);
        void (*startTimer)(void * handle, int msec);
        void (*stopTimer)(void * handle);
    };

    CChannel(HandlePtr handle);

    using Channel::timerEvent;

    // Channel interface
protected:
    virtual MetaObject *metaObject(const Object *object) const override;
    virtual std::string createUuid() const override;
    virtual ProxyObject *createProxyObject(Map &&classinfo) const override;
    virtual void startTimer(int msec) override;
    virtual void stopTimer() override;

private:
    Handle<Callback> * handle_;
    mutable std::map<HandlePtr, CMetaObject*> metaobjs_;
};

struct ChannelStub
{
    void * (*create)(HandlePtr handle);
    void (*registerObject)(void * channel, char const * name, void * object);
    void (*deregisterObject)(void * channel, void * object);
    bool (*blockUpdates)(void * channel);
    void (*setBlockUpdates)(void * channel, bool block);
    void (*connectTo)(void * channel, void * transport, HandlePtr response);
    void (*disconnectFrom)(void * channel, void * transport);
    void (*timerEvent)(void * channel);
    void (*free)(void * channel);
};

extern "C"
{
HYBRIDGEC_EXPORT extern struct ChannelStub channelStub;
}

#endif // CCHANNEL_H
