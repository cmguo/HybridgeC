#include "cproxyobject.h"
#include "cmeta.h"
#include "cchannel.h"
#include "cvariant.h"

#include <core/meta.h>

CProxyObject::CProxyObject(HandlePtr channel)
{
    callback_.callback = &ProxyObjectStub::instance;
    handle_ = cast<CChannel::Callback>(channel)
            ->callback->createProxyObject(channel, cast<void>(&callback_));
}

CProxyObject::~CProxyObject()
{
}

CProxyObject *CProxyObject::fromCallback(HandlePtr callback)
{
    return reinterpret_cast<CProxyObject*>(
                reinterpret_cast<char *>(callback) - offsetof(CProxyObject, callback_));
}

void * CProxyObject::readProperty(const char *property)
{
    MetaObject const * meta = metaObj();
    std::string name = property;
    for (size_t i = 0; i < meta->propertyCount(); ++i) {
        MetaProperty const & mp = meta->property(i);
        if (name == mp.name()) {
            Value v = mp.read(this);
            return CVariant(v, true).detach();
            // TODO: when reset buffer?
        }
    }
    return nullptr;
}

bool CProxyObject::writeProperty(char const * property, void * value)
{
    MetaObject const * meta = metaObj();
    std::string name = property;
    for (size_t i = 0; i < meta->propertyCount(); ++i) {
        MetaProperty const & mp = meta->property(i);
        if (name == mp.name()) {
            return mp.write(this, CVariant(mp.type(), value).toValue());
        }
    }
    return false;
}

bool CProxyObject::invokeMethod(char const * method, void ** args, HandlePtr onResult)
{
    MetaObject const * meta = metaObj();
    std::string name = method;
    for (size_t i = 0; i < meta->methodCount(); ++i) {
        MetaMethod const & md = meta->method(i);
        if (name == md.name()) {
            Array argv;
            for (size_t j = 0; j < md.parameterCount(); ++j)
                argv.emplace_back(CVariant(md.parameterType(j), args[j]).toValue());
            return md.invoke(this, std::move(argv),
                                  [onResult] (Value && result) {
                cast<ResultCallback>(onResult)->callback->apply(onResult, CVariant(result));
            });
        }
    }
    return false;
}

static void handleSignal(void * handler, Object const * object, size_t index, Array && args)
{
    auto callback = reinterpret_cast<Handle<CProxyObject::SignalCallback>*>(handler);
    CVariantArgs argv(std::move(args));
    callback->callback->apply(reinterpret_cast<HandlePtr>(handler), object, index, argv);
}

bool CProxyObject::connect(size_t signalIndex, HandlePtr handler)
{
    MetaObject const * meta = metaObj();
    size_t index = static_cast<size_t>(signalIndex);
    if (index >= meta->methodCount())
        return false;
    MetaMethod const & md = meta->method(index);
    if (!md.isSignal())
        return false;
    return meta->connect(MetaObject::Connection(this, index,
                                         handler, handleSignal));
}

bool CProxyObject::disconnect(size_t signalIndex, HandlePtr handler)
{
    MetaObject const * meta = metaObj();
    size_t index = static_cast<size_t>(signalIndex);
    if (index >= meta->methodCount())
        return false;
    MetaMethod const & md = meta->method(index);
    if (!md.isSignal())
        return false;
    return meta->disconnect(MetaObject::Connection(this, index,
                                         handler, handleSignal));
}

/* ProxyObjectStub */

static const char *metaData(HandlePtr)
{
    return nullptr;
}

static void * readProperty(HandlePtr object, char const * property)
{
    return CProxyObject::fromCallback(object)->readProperty(property);
}

static size_t writeProperty(HandlePtr object, char const * property, void * value)
{
    return CProxyObject::fromCallback(object)->writeProperty(property, value);
}

static size_t invokeMethod(HandlePtr object, char const * method, void ** args, HandlePtr response)
{
    return CProxyObject::fromCallback(object)->invokeMethod(method, args, response);
}

static size_t connect(HandlePtr object, size_t signalIndex, HandlePtr handler)
{
    return CProxyObject::fromCallback(object)->connect(signalIndex, handler);
}

static size_t disconnect(HandlePtr object, size_t signalIndex, HandlePtr handler)
{
    return CProxyObject::fromCallback(object)->disconnect(signalIndex, handler);
}

ProxyObjectStub ProxyObjectStub::instance = {
    ::metaData,
    ::readProperty,
    ::writeProperty,
    ::invokeMethod,
    ::connect,
    ::disconnect
};
