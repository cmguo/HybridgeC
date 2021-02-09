#include "ctransport.h"
#include "chandle.h"

#include <iostream>

extern CTransportStub transportStub;

CTransport::CTransport(CHandlePtr handle)
    : handle_(reinterpret_cast<CHandle<Callback>*>(handle))
{
    stub_.callback = &transportStub;
}

CHandlePtr CTransport::stub()
{
    return cast<void>(&stub_);
}

CTransport * CTransport::fromCallback(CHandlePtr callback)
{
    return reinterpret_cast<CTransport*>(reinterpret_cast<char *>(callback) - offsetof(CTransport, stub_));
}

void CTransport::sendMessage(const Message &message)
{
    handle_->callback->sendMessage(cast<void>(handle_), Value::toJson(message).c_str());
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

static void messageReceived(CHandlePtr transport, char const * message)
{
    CTransport::fromCallback(transport)->messageReceived(message);
}

static void freeTransport(CHandlePtr transport)
{
    delete CTransport::fromCallback(transport);
}

CTransportStub transportStub = {
    messageReceived,
    freeTransport
};
