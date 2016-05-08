#ifndef _SERVER_SESSION_HPP_
#define _SERVER_SESSION_HPP_

#include <string>

#include <schemas/core.h>

namespace session {
    
    typedef fbs::Session fbs_type;
    typedef flatbuffers::Offset<fbs_type> fbs_offset_type;
    
    extern const char* SESSION_KEY_USERNAME;
    
    fbs_offset_type NewSession(flatbuffers::FlatBufferBuilder&, const std::string&);
    
    typedef std::string key_type;
    
    bool IsSessionExist(const fbs_type&);
    void RemoveSession(const fbs_type&);
    
    std::string GetStringValue(const fbs_type&, const key_type&);
    void PutStringValue(const fbs_type&, const key_type&, std::string&);
    
    int GetIntValue(const fbs_type&, const key_type&);
    void PutIntValue(const fbs_type&, const key_type&, int);
    
    long GetLongValue(const fbs_type&, const key_type&);
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