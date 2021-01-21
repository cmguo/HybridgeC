#include "cchannel.h"
#include "cproxyobject.h"
#include "ctransport.h"
#include "cvariant.h"
#include "handleptr.h"

#include <iostream>

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
        std::cout << "CChannel metaObject " << m->className() << std::endl;
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
    CChannel * c = new CChannel(handle);
    std::cout << "createChannel: " << handle << " -> " << c << std::endl;
    return c;
}

static void registerObject(void * channel, char const * name, void * object)
{
    std::cout << "registerObject: " << channel << " " << name << std::endl;
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
    std::cout << "connectTo: " << channel << " "  << transport << std::endl;
    MetaMethod::Response r;
    if (response)
        r = [response] (Value && result) {
            auto r = reinterpret_cast<Handle<CProxyObject::ResultCallback>*>(response);
            r->callback->apply(response, CVariant(result));
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
