#ifndef _SERVER_TYPES_HPP_
#define _SERVER_TYPES_HPP_

#include <string>
#include <functional>
#include <cstdint>

extern "C"{
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

#include <boost/property_tree/ptree.hpp>

#include <schemas/core.h>

typedef uint8_t byte_t;
typedef int8_t sbyte_t;

#ifndef RECV_BUFFER_SIZE
#define RECV_BUFFER_SIZE (2 * (1 << 10)) //2KB
#endif

typedef std::function<ssize_t(const byte_t*, size_t)> ResponseWriter;

inline void SendStatusResponse(fbs::Status status, const ResponseWriter& resp_writer){
    flatbuffers::FlatBufferBuilder builder;
    auto general_resp = fbs::CreateGeneralResponse(builder, 0, status);
    fbs::FinishGeneralResponseBuffer(builder, general_resp);
    
    resp_writer(builder.GetBufferPointer(), builder.GetSize());
}

#define PATH_SEPARATOR '/'

#define ACCOUNT_FILE_NAME "accounts.json"
#define POST_FILE_NAME "post.json"

inline boost::property_tree::path GetPath(const std::string &path){
    return boost::property_tree::path(path, PATH_SEPARATOR);
}

#define PROFILE_PASSWORD_KEY    "password"
#define PROFILE_NIKNAME_KEY     "nickname"

class RequestWrapper {
    
public:
    
    typedef struct sockaddr_in SocketAddrIn;
    
    RequestWrapper(const fbs::RequestPacket& packet_,
                   const SocketAddrIn& client_addr_ = SocketAddrIn()) :
    client_addr(client_addr_),
    raw_packet(packet_){}
    
    const flatbuffers::String* path()const{ return raw_packet.path(); }
    const flatbuffers::Vector<int8_t>* payload()const{ return raw_packet.payload(); }
    
    const char* GetClientAddrStr() const {
        return const_cast<const char*>(inet_ntoa(client_addr.sin_addr));
    }
    int GetClientPort() const {
        return ntohs(client_addr.sin_port);
    }
    
    const SocketAddrIn& GetRawSockAddr() const { return client_addr; }
    
private:
    
    const fbs::RequestPacket& raw_packet;
    
    const SocketAddrIn& client_addr;
};

typedef RequestWrapper Request;

#endif