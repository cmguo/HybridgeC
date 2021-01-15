#include "cproxyobject.h"
#include "cmeta.h"
#include "cchannel.h"
#include "cvariant.h"

#include <core/meta.h>

CProxyObject::CProxyObject(HandlePtr channel)
{
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

bool CProxyObject::getProperty(const char *property, void *value)
{
    MetaObject const * meta = metaObj();
    std::string name = property;
    for (size_t i = 0; i < meta->propertyCount(); ++i) {
        MetaProperty const & mp = meta->property(i);
        if (name == mp.name()) {
            Value v = mp.read(this);
            CVariant::fromValue(v, value);
            // TODO: when reset buffer?
            return true;
        }
    }
    return false;
}

bool CProxyObject::setProperty(char const * property, void * value)
{
    MetaObject const * meta = metaObj();
    std::string name = property;
    for (size_t i = 0; i < meta->propertyCount(); ++i) {
        MetaProperty const & mp = meta->property(i);
        if (name == mp.name()) {
            return mp.write(this, CVariant::toValue(mp.type(), value));
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
                argv.emplace_back(CVariant::toValue(md.parameterType(j), args[j]));
            return md.invoke(this, std::move(argv),
                                  [onResult] (Value && result) {
                cast<ResultCallback>(onResult)->callback->apply(onResult, result.value());
            });
        }
    }
    return false;
}

static void handleSignal(void * handler, Object const * object, size_t index, Array && args)
{
    auto callback = reinterpret_cast<Handle<CProxyObject::SignalCallback>*>(handler);
    std::vector<void*> argv(args.size());
    for (size_t i = 0; i < args.size(); ++i)
        argv[i] = CVariant::fromValue(args[i]);
    callback->callback->apply(reinterpret_cast<HandlePtr>(handler), object, index, &argv[0]);
    CVariant::resetBuffer();
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

static size_t readProperty(HandlePtr handle, const Object *, char const * property, void * result)
{
    return CProxyObject::fromCallback(handle)->getProperty(property, result);
}

static size_t writeProperty(HandlePtr handle, Object *, char const * property, void * value)
{
    return CProxyObject::fromCallback(handle)->setProperty(property, value);
}

static size_t invokeMethod(HandlePtr handle, Object *, char const * method, void ** args, HandlePtr response)
{
    return CProxyObject::fromCallback(handle)->invokeMethod(method, args, response);
}

ProxyObjectStub ProxyObjectStub::instance = {
    ::metaData,
    ::readProperty,
    ::writeProperty,
    ::invokeMethod
};
