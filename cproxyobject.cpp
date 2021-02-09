#include "cproxyobject.h"
#include "cmetaobject.h"
#include "cchannel.h"
#include "cvariant.h"

#include <core/metaobject.h>

CProxyObject::CProxyObject(CHandlePtr channel, Map &&classinfo)
    : ProxyObject(std::move(classinfo))
{
    callback_.callback = &ProxyObjectStub::instance;
    handle_ = cast<CChannel::Callback>(channel)
            ->callback->createProxyObject(channel, cast<void>(&callback_));
}

CProxyObject::~CProxyObject()
{
}

CProxyObject *CProxyObject::fromCallback(CHandlePtr callback)
{
    return reinterpret_cast<CProxyObject*>(
                reinterpret_cast<char *>(callback) - offsetof(CProxyObject, callback_));
}

char const * CProxyObject::metaData()
{
    Map data = CMetaObject::encode(*metaObj());
    std::string str = Value::toJson(data);
    void * buffer = CVariant::allocBuffer(str.length() + 1);
    memcpy(buffer, str.c_str(), str.length() + 1);
    return reinterpret_cast<char const *>(buffer);
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

bool CProxyObject::invokeMethod(char const * method, void ** args, CHandlePtr onResult)
{
    MetaObject const * meta = metaObj();
    std::string name = method;
    CVariant argsBuffer(Value::None, args);
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
    auto callback = reinterpret_cast<CHandle<CProxyObject::SignalCallback>*>(handler);
    CVariantArgs argv(std::move(args));
    callback->callback->apply(reinterpret_cast<CHandlePtr>(handler), object, index, argv);
}

bool CProxyObject::connect(size_t signalIndex, CHandlePtr handler)
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

bool CProxyObject::disconnect(size_t signalIndex, CHandlePtr handler)
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

static const char *metaData(CHandlePtr object)
{
    return CProxyObject::fromCallback(object)->metaData();
}

static void * readProperty(CHandlePtr object, char const * property)
{
    return CProxyObject::fromCallback(object)->readProperty(property);
}

static size_t writeProperty(CHandlePtr object, char const * property, void * value)
{
    return CProxyObject::fromCallback(object)->writeProperty(property, value);
}

static size_t invokeMethod(CHandlePtr object, char const * method, void ** args, CHandlePtr response)
{
    return CProxyObject::fromCallback(object)->invokeMethod(method, args, response);
}

static size_t connect(CHandlePtr object, size_t signalIndex, CHandlePtr handler)
{
    return CProxyObject::fromCallback(object)->connect(signalIndex, handler);
}

static size_t disconnect(CHandlePtr object, size_t signalIndex, CHandlePtr handler)
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
