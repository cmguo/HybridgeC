#include "cmetaobject.h"
#include "cvariant.h"
#include "chandle.h"
#include "priv/collection.h"

#include <iostream>

/* CMetaObject */

CMetaObject::CMetaObject()
{
}

CMetaObject::CMetaObject(CMetaObject * super, CHandle<Callback> * handle)
    : super_(super)
    , handle_(handle)
{
    char const * meta = handle_->callback->metaData(cast<void>(handle_));
    metaData_.swap(Value::fromJson(meta).toMap(metaData_));
    for (auto & p : mapValue(metaData_, "properties").toArray()) {
        properties_.push_back(CMetaProperty(p.toArray(), handle));
    }
    // destroyed signal, index: -1, because +1
    static Value destroyed = Value::fromJson("[\"destroyed\", 5, -1, \"V()\", 9, [], []]");
    methods_.push_back(CMetaMethod(destroyed.toArray()));
    for (auto & m : mapValue(metaData_, "methods").toArray()) {
        methods_.push_back(CMetaMethod(m.toArray(), handle));
    }
    for (auto & e : mapValue(metaData_, "enums").toArray()) {
        enums_.push_back(CMetaEnum(e.toArray()));
    }
}

Map CMetaObject::encode(const MetaObject &meta)
{
    Array properties;
    for (size_t i = 0; i < meta.propertyCount(); ++i) {
        properties.emplace_back(CMetaProperty::encode(meta.property(i)));
    }
    Array methods;
    for (size_t i = 0; i < meta.methodCount(); ++i) {
        methods.emplace_back(CMetaMethod::encode(meta.method(i)));
    }
    Array enumerators;
    for (size_t i = 0; i < meta.enumeratorCount(); ++i) {
        enumerators.emplace_back(CMetaEnum::encode(meta.enumerator(i)));
    }
    Map data;
    data.emplace("class", meta.className());
    data.emplace("properties", std::move(properties));
    data.emplace("methods", std::move(methods));
    data.emplace("enumerators", std::move(enumerators));
    return data;
}

const char *CMetaObject::className() const
{
    return mapValue(metaData_, "class").toString().c_str();
}

size_t CMetaObject::propertyCount() const
{
    return super_->propertyCount() + properties_.size();
}

const MetaProperty &CMetaObject::property(size_t index) const
{
    size_t n = super_->propertyCount();
    return index < n ? super_->property(index) : properties_.at(index);
}

size_t CMetaObject::methodCount() const
{
    return super_->methodCount() + methods_.size();
}

const MetaMethod &CMetaObject::method(size_t index) const
{
    size_t n = super_->methodCount();
    return index < n ? super_->method(index) : methods_.at(index);
}

size_t CMetaObject::enumeratorCount() const
{
    return super_->enumeratorCount() + enums_.size();
}

const MetaEnum &CMetaObject::enumerator(size_t index) const
{
    size_t n = super_->propertyCount();
    return index < n ? super_->enumerator(index) : enums_.at(index);
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

/* CMetaProperty */

CMetaProperty::CMetaProperty(Array const & metaData, CHandle<CMetaObject::Callback> * handle)
    : metaData_(metaData)
    , handle_(handle)
{
}

Array CMetaProperty::encode(const MetaProperty &prop)
{
    int flag = 0;
    if (prop.isConstant()) flag |= CMetaObject::Constant;
    if (prop.hasNotifySignal()) flag |= CMetaObject::Signal;
    Array data;

    data.emplace_back(prop.name());
    data.emplace_back(flag);
    data.emplace_back(-1);
    data.emplace_back(static_cast<int>(prop.type()));
    data.emplace_back(static_cast<int>(prop.notifySignalIndex()));
    return data;
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
    return static_cast<Value::Type>(metaData_[3].toInt());
}

bool CMetaProperty::isConstant() const
{
    return metaData_[1].toInt() & CMetaObject::Constant;
}

size_t CMetaProperty::propertyIndex() const
{
    return static_cast<size_t>(metaData_[2].toInt());
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
    size_t index = static_cast<size_t>(metaData_[2].toInt());
    CVariant value = {type(), handle_->callback->readProperty(cast<void>(handle_), object, index)};
    return value.toValue(true);
}

bool CMetaProperty::write(Object *object, Value &&value) const
{
    size_t index = static_cast<size_t>(metaData_[2].toInt());
    return handle_->callback->writeProperty(cast<void>(handle_), object, index, CVariant(value));
}

/* CMetaMethod */

CMetaMethod::CMetaMethod(Array const & metaData, CHandle<CMetaObject::Callback> * handle)
    : metaData_(metaData)
    , handle_(handle)
{
}

Array CMetaMethod::encode(const MetaMethod &method)
{
    Array parameterTypes;
    Array parameterNames;
    for (size_t i = 0; i < method.parameterCount(); ++i){
        parameterTypes.emplace_back(method.parameterType(i));
        parameterNames.emplace_back(method.parameterName(i));
    }
    int flag = 0;
    if (method.isPublic()) flag |= CMetaObject::Public;
    if (method.isSignal()) flag |= CMetaObject::Signal;
    Array data;
    data.emplace_back(method.name());
    data.emplace_back(flag);
    data.emplace_back(static_cast<int>(method.methodIndex()));
    data.emplace_back(method.methodSignature());
    data.emplace_back(static_cast<int>(method.returnType()));
    data.emplace_back(std::move(parameterTypes));
    data.emplace_back(std::move(parameterNames));
    return data;
}

const char *CMetaMethod::name() const
{
    return metaData_[0].toString().c_str();
}

bool CMetaMethod::isValid() const
{
    return true;
}

bool CMetaMethod::isSignal() const
{
    return metaData_[1].toInt() & CMetaObject::Signal;
}

bool CMetaMethod::isPublic() const
{
    return metaData_[1].toInt() & CMetaObject::Public;
}

size_t CMetaMethod::methodIndex() const
{
    return static_cast<size_t>(metaData_[2].toInt() + 1);
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

struct RespCHandle : CHandle<void>
{
    RespCHandle(MetaMethod::Response const &resp, Value::Type resultType)
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
        reinterpret_cast<RespCHandle*>(resp)->invoke(result);
    }
private:
    MetaMethod::Response resp_;
    Value::Type resultType_;
};

bool CMetaMethod::invoke(Object *object, Array &&args, const MetaMethod::Response &resp) const
{
    CVariantArgs argv(std::move(args));
    void * result = handle_->callback
            ->invokeMethod(cast<void>(handle_), object, methodIndex() - 1, argv);
    resp(CVariant(returnType(), result).toValue());
    return result;
}

/* CMetaEnum */

CMetaEnum::CMetaEnum(Array const & metaData)
    : metaData_(metaData)
{
}

Array CMetaEnum::encode(const MetaEnum &enumerator)
{
    Array keys;
    Array values;
    for (size_t i = 0; i < enumerator.keyCount(); ++i){
        keys.emplace_back(enumerator.key(i));
        values.emplace_back(enumerator.value(i));
    }
    Array data;
    data.emplace_back(enumerator.name());
    data.emplace_back(std::move(keys));
    data.emplace_back(std::move(values));
    return data;
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
