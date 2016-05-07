#ifndef _SERVER_ROUTER_HPP_
#define _SERVER_ROUTER_HPP_

#include <string>
#include <functional>

extern "C"{
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

#include <Log.hpp>
#include "Types.hpp"

#include <schemas/packet_generated.h>
#include <boost/property_tree/ptree.hpp>

//typedef fbs::RequestPacket Request;

class RequestWrapper {
    
public:
    
    typedef struct sockaddr_in SocketAddrIn;
    
    RequestWrapper(const fbs::RequestPacket& packet_,
                   const SocketAddrIn& client_addr_ = SocketAddrIn()) :
        client_addr(client_addr_),
        packet(packet_){}
    
    const flatbuffers::String* path()const{ return packet.path(); }
    const flatbuffers::Vector<int8_t>* payload()const{ return packet.payload(); }
    
    const char* GetClientAddrStr(){
        return const_cast<const char*>(inet_ntoa(client_addr.sin_addr));
    }
    int GetClientPort(){
        return ntohs(client_addr.sin_port);
    }
    
private:
    
    const fbs::RequestPacket& packet;
    
    const SocketAddrIn& client_addr;
};

typedef RequestWrapper Request;

#define RequestPathStr(req) \
    (req).path()->str()

typedef std::function<void(const Request&,const ResponseWriter&)> HandleFunc;
#define HANDLE_FUNC() \
    [](const Request& request, const ResponseWriter& response_writer)->void

class Router {
    
public:
    
    static const HandleFunc NotFoundHandler;
    
    Router(){/*Default construtor*/}
    Router(const Router& that){
        //Copy constructor
        callback_tree = that.callback_tree;
    }
    
    void Process(const Request&, const ResponseWriter&);
    
    //Handler function
    Router& Path(std::string, HandleFunc);
    
    //Sub-router
    Router& Path(std::string, const Router&);
    
private:
    
    boost::property_tree::basic_ptree<std::string, HandleFunc> callback_tree;
    
    
};

#endif