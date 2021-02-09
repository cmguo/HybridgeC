#ifndef CTRANSPORT_H
#define CTRANSPORT_H

#include "HybridgeC_global.h"
#include "chandle.h"

struct CTransportCallback
{
    void (*sendMessage)(CHandlePtr handle, char const *message);
};

#ifdef __cplusplus

#include <core/transport.h>

struct CTransportStub;

class CTransport : public Transport
{
public:
    typedef CTransportCallback Callback;
    
    CTransport(CHandlePtr handle);

public:
    CHandlePtr stub();
    
    static CTransport * fromCallback(CHandlePtr callback);

    // Transport interface
public:
    virtual void sendMessage(const Message &message) override;

    void messageReceived(std::string const & message);

private:
    CHandle<CTransportStub> stub_;
    CHandle<Callback> * handle_;
};

#endif

struct CTransportStub
{
    void (*messageReceived)(CHandlePtr transport, char const * message);
    void (*free)(CHandlePtr transport);
};

#endif // CTRANSPORT_H
