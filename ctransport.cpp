#include "ctransport.h"
#include "handleptr.h"

#include <iostream>

CTransport::CTransport(HandlePtr handle)
    : handle_(reinterpret_cast<Handle<Callback>*>(handle))
{
}

void CTransport::sendMessage(const Message &message)
{
    handle_->callback->sendMessage(handle_, Value::toJson(message).c_str());
}

void CTransport::messageReceived(const std::string &message)
{
    std::cout << "messageReceived: " << message << std::endl;
    Map empty;
    Value v = Value::fromJson(message);
    Transport::messageReceived(std::move(v.toMap(empty)));
}

/* TransportStub */

#define T reinterpret_cast<CTransport *>(transport)

static void * createTransport(HandlePtr handle)
{
    CTransport * t = new CTransport(handle);
    std::cout << "createTransport: " << handle << " -> " << t << std::endl;
    return t;
}

static void messageReceived(void * transport, char const * message)
{
    T->messageReceived(message);
}

static void freeTransport(void * transport)
{
    delete T;
}


extern "C"
{
    HYBRIDGEC_EXPORT extern struct TransportStub transportStub = {
        createTransport,
        messageReceived,
        freeTransport
    };

}
