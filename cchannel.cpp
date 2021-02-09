#include "cchannel.h"
#include "cproxyobject.h"
#include "ctransport.h"
#include "cvariant.h"
#include "chandle.h"

#include <iostream>

CChannel::CChannel(CHandlePtr handle)
    : handle_(cast<Callback>(handle))
{
}

CHandlePtr CChannel::stub()
{
    return cast<void>(&stub_);
}

CChannel * CChannel::fromCallback(CHandlePtr callback)
{
    return reinterpret_cast<CChannel*>(reinterpret_cast<char *>(callback) - offsetof(CChannel, stub_));
}

MetaObject *CChannel::metaObject(const Object *object) const
{
    CHandlePtr h = handle_->callback->metaObject(cast<void>(handle_), object);
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
    char const * uuid = handle_->callback->createUuid(cast<void>(handle_));
    std::string str(uuid);
    ::free(const_cast<char *>(uuid));
    return str;
}

ProxyObject *CChannel::createProxyObject(Map &&classinfo) const
{
    CProxyObject * po = new CProxyObject(cast<void>(handle_), std::move(classinfo));
    return po;
}

void CChannel::startTimer(int msec)
{
    handle_->callback->startTimer(cast<void>(handle_), msec);
}

void CChannel::stopTimer()
{
    handle_->callback->stopTimer(cast<void>(handle_));
}

/* ChannelStub */

static void registerObject(CHandlePtr channel, char const * name, void * object)
{
    std::cout << "registerObject: " << channel << " " << name << std::endl;
    return CChannel::fromCallback(channel)->registerObject(name, object);
}

static void deregisterObject(CHandlePtr channel, void * object)
{
    return CChannel::fromCallback(channel)->deregisterObject(object);
}

static bool blockUpdates(CHandlePtr channel)
{
    return CChannel::fromCallback(channel)->blockUpdates();
}

static void setBlockUpdates(CHandlePtr channel, bool block)
{
    return CChannel::fromCallback(channel)->setBlockUpdates(block);
}

static void connectTo(CHandlePtr channel, void * transport, CHandlePtr response)
{
    std::cout << "connectTo: " << channel << " "  << transport << std::endl;
    MetaMethod::Response r;
    if (response)
        r = [response] (Value && result) {
            auto r = reinterpret_cast<CHandle<CProxyObject::ResultCallback>*>(response);
            r->callback->apply(response, CVariant(result));
        };
    return CChannel::fromCallback(channel)->connectTo(
                reinterpret_cast<CTransport*>(transport), r);
}

static void disconnectFrom(CHandlePtr channel, void * transport)
{
    return CChannel::fromCallback(channel)->disconnectFrom(reinterpret_cast<CTransport*>(transport));
}

static void timerEvent(CHandlePtr channel)
{
    return CChannel::fromCallback(channel)->timerEvent();
}

static void freeChannel(CHandlePtr channel)
{
    delete CChannel::fromCallback(channel);
}

extern "C"
{
    HYBRIDGEC_EXPORT struct CChannelStub channelStub = {
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
