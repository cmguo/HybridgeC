#include "cmeta.h"
#include "cvariant.h"
#include "handleptr.h"
#include "priv/collection.h"

CMetaObject::CMetaObject(HandlePtr handle)
    : handle_(cast<Callback>(handle))
{
    char const * meta = handle_->callback->metaData(handle_);
    metaData_.swap(Value::fromJson(meta).toMap(metaData_));
}

const char *CMetaObject::className() const
{
    return mapValue(metaData_, "class").toString().c_str();
}

size_t CMetaObject::propertyCount() const
{
    return properties_.size();
}

const MetaProperty &CMetaObject::property(size_t index) const
{
    return properties_.at(index);
}

size_t CMetaObject::methodCount() const
{
    return methods_.size();
}

const MetaMethod &CMetaObject::method(size_t index) const
{
    return methods_.at(index);
}

size_t CMetaObject::enumeratorCount() const
{
    return enums_.size();
}

const MetaEnum &CMetaObject::enumerator(size_t index) const
{
    return enums_.at(index);
}

bool CMetaObject::connect(const MetaObject::Connection &c) const
{
    if (c.signalIndex() == 0)
        return true;
    return false;
}

bool CMetaObject::disconnect(const MetaObject::Connection &c) const
{
    return c;
}

CMetaProperty::CMetaProperty(Array const & metaData, HandlePtr handle)
    : metaData_(metaData)
    , handle_(cast<CMetaObject::Callback>(handle))
{
}

const char *CMetaProperty::name() const
{
    return metaData_[0].toString().c_str();
}

bool CMetaProperty::isValid() const
{
    return handle_;
}

Value::Type CMetaProperty::type() const
{
    return static_cast<Value::Type>(metaData_[1].toInt());
}

bool CMetaProperty::isConstant() const
{
    return metaData_[1].toInt() & CMetaObject::Constant;
}

bool CMetaProperty::hasNotifySignal() const
{
    return metaData_[1].toInt() & CMetaObject::Signal;
}

size_t CMetaProperty::notifySignalIndex() const
{
    return static_cast<size_t>(metaData_[4].toInt());
}

const MetaMethod &CMetaProperty::notifySignal() const
{
    Array a;
    static CMetaMethod m(a);
    return m;
}

Value CMetaProperty::read(const Object *object) const
{
    char buf[32]; // Enought for all types
    Value result(type(), buf);
    size_t index = static_cast<size_t>(metaData_[3].toInt());
    handle_->callback->readProperty(handle_, object, index, buf);
    return result;
}

bool CMetaProperty::write(Object *object, Value &&value) const
{
    size_t index = static_cast<size_t>(metaData_[3].toInt());
    return handle_->callback->writeProperty(handle_, object, index, value.value());
}

CMetaMethod::CMetaMethod(Array const & metaData, HandlePtr handle)
    : metaData_(metaData)
    , handle_(cast<CMetaObject::Callback>(handle))
{
}

const char *CMetaMethod::name() const
{
    return metaData_[0].toString().c_str();
}

bool CMetaMethod::isValid() const
{
    return handle_;
}

bool CMetaMethod::isSignal() const
{
    return metaData_[12].toInt() & CMetaObject::Signal;
}

bool CMetaMethod::isPublic() const
{
    return metaData_[1].toInt() & CMetaObject::Public;
}

size_t CMetaMethod::methodIndex() const
{
    return static_cast<size_t>(metaData_[2].toInt());
}

const char *CMetaMethod::methodSignature() const
{
    return metaData_[3].toString().c_str();
}

Value::Type CMetaMethod::returnType() const
{
    return static_cast<Value::Type>(metaData_[4].toInt());
}

size_t CMetaMethod::parameterCount() const
{
    return metaData_[5].toArray().size();
}

Value::Type CMetaMethod::parameterType(size_t index) const
{
    return static_cast<Value::Type>(metaData_[5].toArray().at(index).toInt());
}

const char *CMetaMethod::parameterName(size_t index) const
{
    return metaData_[6].toArray().at(index).toString().c_str();
}

struct RespHandle : Handle<void>
{
    RespHandle(MetaMethod::Response const &resp, Value::Type resultType)
    {
        callback = reinterpret_cast<void *>(&invokeResponse);
        resp_ = resp;
        resultType_ = resultType;
    }
    void invoke(void * result)
    {
        Value v(resultType_, result);
        resp_(std::move(v));
    }
    static void invokeResponse(void * resp, void * result)
    {
        reinterpret_cast<RespHandle*>(resp)->invoke(result);
    }
private:
    MetaMethod::Response resp_;
    Value::Type resultType_;
};

bool CMetaMethod::invoke(Object *object, Array &&args, const MetaMethod::Response &resp) const
{
    std::vector<void*> argv(args.size());
    for (size_t i = 0; i < args.size(); ++i)
        argv[i] = CVariant::fromValue(args[i]);
    char buf[32]; // Enought for all types
    bool ok = handle_->callback->invokeMethod(handle_, object, methodIndex(), &argv[0], buf);
    CVariant::resetBuffer();
    if (ok)
        resp(CVariant::toValue(returnType(), buf));
    return ok;
}


CMetaEnum::CMetaEnum(Array const & metaData)
    : metaData_(metaData)
{
}

const char *CMetaEnum::name() const
{
    return metaData_[0].toString().c_str();
}

size_t CMetaEnum::keyCount() const
{
    return metaData_[1].toArray().size();
}

const char *CMetaEnum::key(size_t index) const
{
    return metaData_[1].toArray().at(index).toString().c_str();
}

int CMetaEnum::value(size_t index) const
{
    return metaData_[2].toArray().at(index).toInt();
}
