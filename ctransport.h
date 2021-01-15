#ifndef CTRANSPORT_H
#define CTRANSPORT_H

#include "HybridgeC_global.h"
#include "handleptr.h"

#include <core/transport.h>

class CTransport : public Transport
{
public:
    struct Callback
    {
        void (*sendMessage)(void * handle, char const *message);
    };

    CTransport(HandlePtr handle);

    // Transport interface
public:
    virtual void sendMessage(const Message &message) override;

    void messageReceived(std::string const & message);

private:
    Handle<Callback> * handle_;
};

struct TransportStub
{
    void * (*create)(HandlePtr handle);
    void (*messageReceived)(void * transport, char const * message);
    void (*free)(void * transport);
};

extern "C"
{
    HYBRIDGEC_EXPORT extern struct TransportStub transportStub;
}

#endif // CTRANSPORT_H
