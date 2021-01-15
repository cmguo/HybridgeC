#include "cchannel.h"
#include "cproxyobject.h"
#include "ctransport.h"
#include "handleptr.h"

CChannel::CChannel(HandlePtr handle)
    : handle_(reinterpret_cast<Handle<Callback>*>(handle))
{
}

MetaObject *CChannel::metaObject(const Object *object) const
{
    HandlePtr h = handle_->callback->metaObject(handle_, object);
    auto it = metaobjs_.find(h);
    if (it == metaobjs_.end()) {
        CMetaObject * m = new CMetaObject(h);
        it = metaobjs_.insert(std::make_pair(h, m)).first;
    }
    return it->second;
}

std::string CChannel::createUuid() const
{
    char const * uuid = handle_->callback->createUuid(handle_);
    std::string str(uuid);
    ::free(const_cast<char *>(uuid));
    return str;
}

ProxyObject *CChannel::createProxyObject() const
{
    CProxyObject * po = new CProxyObject(cast<void>(handle_));
    return po;
}

void CChannel::startTimer(int msec)
{
    handle_->callback->startTimer(handle_, msec);
}

void CChannel::stopTimer()
{
    handle_->callback->stopTimer(handle_);
}

/* ChannelStub */

#define C reinterpret_cast<CChannel *>(channel)

static void * createChannel(HandlePtr handle)
{
    return new CChannel(handle);
}

static void registerObject(void * channel, char const * name, void * object)
{
    return C->registerObject(name, object);
}

static void deregisterObject(void * channel, void * object)
{
    return C->deregisterObject(object);
}

static bool blockUpdates(void * channel)
{
    return C->blockUpdates();
}

static void setBlockUpdates(void * channel, bool block)
{
    return C->setBlockUpdates(block);
}

static void connectTo(void * channel, void * transport, HandlePtr response)
{
    MetaMethod::Response r;
    if (response)
        r = [h = *response] (Value && result) {
            auto r = reinterpret_cast<void (*)(void *)>(h.callback);
            r(result.value());
        };
    return C->connectTo(
                reinterpret_cast<CTransport*>(transport), r);
}

static void disconnectFrom(void * channel, void * transport)
{
    return C->disconnectFrom(reinterpret_cast<CTransport*>(transport));
}

static void timerEvent(void * channel)
{
    return C->timerEvent();
}

static void freeChannel(void * channel)
{
    delete C;
}

extern "C"
{
    HYBRIDGEC_EXPORT struct ChannelStub channelStub = {
        createChannel,
        registerObject,
        deregisterObject,
        blockUpdates,
        setBlockUpdates,
        connectTo,
        disconnectFrom,
        timerEvent,
        freeChannel
    };

}
