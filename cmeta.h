#ifndef CMETAOBJECT_H
#define CMETAOBJECT_H

#include "handleptr.h"

#include <core/meta.h>

class CMetaProperty;
class CMetaMethod;
class CMetaEnum;

class CMetaObject : public MetaObject
{
public:
    struct Callback
    {
        const char *(*metaData)(void const * handle);
        void * (*readProperty)(void const * handle, const Object *object, size_t propertyIndex);
        size_t (*writeProperty)(void const * handle, Object *object, size_t propertyIndex, void * value);
        void * (*invokeMethod)(void const * handle, Object *object, size_t methodIndex, void ** args);
    };

    enum {
        Public = 1,
        Constant = 2,
        Signal = 4, // hasSignal
    };

    CMetaObject(HandlePtr handle);

    // MetaObject interface
public:
    virtual const char *className() const override;
    virtual size_t propertyCount() const override;
    virtual const MetaProperty &property(size_t index) const override;
    virtual size_t methodCount() const override;
    virtual const MetaMethod &method(size_t index) const override;
    virtual size_t enumeratorCount() const override;
    virtual const MetaEnum &enumerator(size_t index) const override;
    virtual bool connect(const Connection &c) const override;
    virtual bool disconnect(const Connection &c) const override;

private:
    Handle<Callback> const * handle_;
    Map metaData_;
    std::vector<CMetaProperty> properties_;
    std::vector<CMetaMethod> methods_;
    std::vector<CMetaEnum> enums_;
};

class CMetaProperty : public MetaProperty
{
public:
    CMetaProperty(Array const & metaData, HandlePtr handle = nullptr);

    // MetaProperty interface
public:
    virtual const char *name() const override;
    virtual bool isValid() const override;
    virtual Value::Type type() const override;
    virtual bool isConstant() const override;
    virtual bool hasNotifySignal() const override;
    virtual size_t notifySignalIndex() const override;
    virtual const MetaMethod &notifySignal() const override;
    virtual Value read(const Object *object) const override;
    virtual bool write(Object *object, Value &&value) const override;

private:
    /*
     * [0] name
     * [1] flags
     * [2] type
     * [3] index
     * [4] notifySignalIndex (optional)
     */
    Array const & metaData_;
    Handle<CMetaObject::Callback> const * handle_;
};

class CMetaMethod : public MetaMethod
{
public:
    CMetaMethod(Array const & metaData, HandlePtr handle = nullptr);

    // MetaMethod interface
public:
    virtual const char *name() const override;
    virtual bool isValid() const override;
    virtual bool isSignal() const override;
    virtual bool isPublic() const override;
    virtual size_t methodIndex() const override;
    virtual const char *methodSignature() const override;
    virtual Value::Type returnType() const override;
    virtual size_t parameterCount() const override;
    virtual Value::Type parameterType(size_t index) const override;
    virtual const char *parameterName(size_t index) const override;
    virtual bool invoke(Object *object, Array &&args, const Response &resp) const override;

private:
    /*
     * [0] name
     * [1] flags
     * [2] index
     * [3] signature
     * [4] returnType
     * [5] parameterTypes
     * [6] parameterNames
     */
    Array const & metaData_;
    Handle<CMetaObject::Callback> const * handle_;
};

class CMetaEnum : public MetaEnum
{
public:
    CMetaEnum(Array const & metaData);

    // MetaEnum interface
public:
    virtual const char *name() const override;
    virtual size_t keyCount() const override;
    virtual const char *key(size_t index) const override;
    virtual int value(size_t index) const override;

private:
    /*
     * [0] name
     * [1] keys
     * [2] values
     */
    Array const & metaData_;
};

#endif // CMETAOBJECT_H
