#ifndef _SERVER_SESSION_HPP_
#define _SERVER_SESSION_HPP_

#include <string>
#include <unordered_map>

#include <schemas/core.h>

#include "Types.hpp"

namespace session {
    
    typedef fbs::Session fbs_type;
    typedef flatbuffers::Offset<fbs_type> fbs_offset_type;
    
    extern const char* SESSION_KEY_USERNAME;
    
    extern const char* SESSION_KEY_TIMESTAMP;
    
    extern const char* SESSION_KEY_CLIENT_ADDR;
    
    extern const char* SESSION_KEY_CAN_CHAT;
    
    fbs_offset_type NewSession(flatbuffers::FlatBufferBuilder&,
                               const std::string&,
                               const Request::SocketAddrIn&);
    
    typedef std::string key_type;
    typedef std::unordered_map<key_type, std::string> value_map_type;
    
    bool IsSessionExist(const fbs_type&);
    void RemoveSession(const fbs_type&);
    
    std::string GetHashByUsername(const std::string&);
    
    const Request::SocketAddrIn& GetClientAddr(const std::string&);
    const Request::SocketAddrIn& GetClientAddr(const fbs_type&);
    
    std::string GetStringValue(const std::string&, const key_type&);
    std::string GetStringValue(const fbs_type&, const key_type&);
    void PutStringValue(const std::string&, const key_type&, std::string&);
    void PutStringValue(const fbs_type&, const key_type&, std::string&);
    
    int GetIntValue(const std::string&, const key_type&);
    int GetIntValue(const fbs_type&, const key_type&);
    void PutIntValue(const std::string&, const key_type&, int);
    void PutIntValue(const fbs_type&, const key_type&, int);
    
    long GetLongValue(const std::string&, const key_type&);
    long GetLongValue(const fbs_type&, const key_type&);
    void PutLongValue(const std::string&, const key_type&, long);
    void PutLongValue(const fbs_type&, const key_type&, long);
    
    struct BadTransformException {
        std::string message;
        std::string cause;
        
        BadTransformException(std::string cause_, std::string msg = "") :
                message(msg),
                cause(cause_){}
    };
    
} //namespace session

#endif