#include "cchannel.h"
#include "cproxyobject.h"
#include "ctransport.h"
#include "cvariant.h"
#include "chandle.h"

#include <iostream>

extern CChannelStub channelStub;

CChannel::CChannel(CHandlePtr handle)
    : handle_(cast<Callback>(handle))
{
    stub_.callback = &channelStub;
    metaobjs_.insert(std::make_pair(nullptr, new CRootMetaObject));
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
    return metaObject(cast<CMetaObject::Callback>(h));
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

CMetaObject *CChannel::metaObject(CHandle<CMetaObject::Callback> *handle) const
{
    auto it = metaobjs_.find(handle);
    if (it == metaobjs_.end()) {
        CHandle<CMetaObject::Callback> * super = handle->callback->super
                ? reinterpret_cast<CHandle<CMetaObject::Callback>*>(handle->callback->super(cast<void>(handle)))
                : nullptr;
        CMetaObject * m = new CMetaObject(metaObject(super), handle);
        std::cout << "CChannel metaObject " << m->className() << std::endl;
        it = metaobjs_.insert(std::make_pair(handle, m)).first;
    }
    return it->second;
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

static void connectTo(CHandlePtr channel, CHandlePtr transport, CHandlePtr response)
{
    std::cout << "connectTo: " << channel << " "  << transport << std::endl;
    MetaMethod::Response r;
    if (response)
        r = [response] (Value && result) {
            auto r = reinterpret_cast<CHandle<CProxyObject::ResultCallback>*>(response);
            r->callback->apply(response, CVariant(result));
        };
    return CChannel::fromCallback(channel)->connectTo(
                CTransport::fromCallback(transport), r);
}

static void disconnectFrom(CHandlePtr channel, CHandlePtr transport)
{
    return CChannel::fromCallback(channel)->disconnectFrom(CTransport::fromCallback(transport));
}

static void timerEvent(CHandlePtr channel)
{
    return CChannel::fromCallback(channel)->timerEvent();
}

static void freeChannel(CHandlePtr channel)
{
    delete CChannel::fromCallback(channel);
}

CChannelStub channelStub = {
    registerObject,
    deregisterObject,
    blockUpdates,
    setBlockUpdates,
    connectTo,
    disconnectFrom,
    timerEvent,
    freeChannel
};

