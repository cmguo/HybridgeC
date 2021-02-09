#ifndef CMETAOBJECT_H
#define CMETAOBJECT_H

#include "chandle.h"

#include <stdlib.h>

struct CMetaObjectCallback
{
    void * (*super)(CHandlePtr handle);
    const char *(*metaData)(CHandlePtr handle);
    void * (*readProperty)(CHandlePtr handle, const void *object, size_t propertyIndex);
    size_t (*writeProperty)(CHandlePtr handle, void *object, size_t propertyIndex, void * value);
    void * (*invokeMethod)(CHandlePtr handle, void *object, size_t methodIndex, void ** args);
};

#ifdef __cplusplus

#include <core/metaobject.h>

class CMetaProperty;
class CMetaMethod;
class CMetaEnum;

class CMetaObject : public MetaObject
{
public:
    typedef CMetaObjectCallback Callback;
    
    enum {
        Public = 1,
        Constant = 2,
        Signal = 4, // hasSignal
    };

    CMetaObject(CMetaObject * super, CHandle<Callback> * handle);

    static Map encode(MetaObject const & meta);

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

protected:
    CMetaObject();

private:
    CMetaObject * super_ = nullptr;
    CHandle<Callback> * handle_ = nullptr;
    Map metaData_;
    std::vector<CMetaProperty> properties_;
    std::vector<CMetaMethod> methods_;
    std::vector<CMetaEnum> enums_;
};

class CRootMetaObject : public CMetaObject
{
public:
    CRootMetaObject() {}

    // MetaObject interface
public:
    virtual size_t propertyCount() const override { return 0; }
    virtual size_t methodCount() const override { return 0; }
    virtual size_t enumeratorCount() const override { return 0; }
};

class CMetaProperty : public MetaProperty
{
public:
    CMetaProperty(Array const & metaData, CHandle<CMetaObject::Callback> * handle = nullptr);

    static Array encode(MetaProperty const & prop);

    // MetaProperty interface
public:
    virtual const char *name() const override;
    virtual bool isValid() const override;
    virtual Value::Type type() const override;
    virtual bool isConstant() const override;
    virtual size_t propertyIndex() const override;
    virtual bool hasNotifySignal() const override;
    virtual size_t notifySignalIndex() const override;
    virtual const MetaMethod &notifySignal() const override;
    virtual Value read(const Object *object) const override;
    virtual bool write(Object *object, Value &&value) const override;

private:
    /*
     * [0] name
     * [1] flags
     * [2] index
     * [3] type
     * [4] notifySignalIndex (optional)
     */
    Array const & metaData_;
    CHandle<CMetaObject::Callback> * handle_;
};

class CMetaMethod : public MetaMethod
{
public:
    CMetaMethod(Array const & metaData, CHandle<CMetaObject::Callback> * handle = nullptr);

    static Array encode(MetaMethod const & method);

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
    CHandle<CMetaObject::Callback> * handle_;
};

class CMetaEnum : public MetaEnum
{
public:
    CMetaEnum(Array const & metaData);

    static Array encode(MetaEnum const & enumerator);

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

#endif // __cplusplus

#endif // CMETAOBJECT_H
