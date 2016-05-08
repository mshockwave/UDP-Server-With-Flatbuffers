#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <string>
#include <vector>
#include <functional>

#include <schemas/core.h>

#ifndef RECV_BUFFER_SIZE
#define RECV_BUFFER_SIZE (2 * (1 << 10)) //2KB
#endif

namespace utils{
    
    inline void TrimString(std::string &str, char ch = ' '){
        int i;
        //Front
        for(i = 0; i < str.length(); i++){
            if(str[i] != ch) break;
        }
        if(i > 0) str.erase(0, i);
        
        //Tail
        for(i = str.length() - 1; i >= 0; i--){
            if(str[i] != ch) break;
        }
        if(i + 1 < str.length()) str.erase(i+1);
    }
    
    typedef std::function<void(void)> FinalizeCallback;
    //Default: Push to the front
    void AddFinalizeCallback(const FinalizeCallback&);
    void PushBackFinalizeCallback(const FinalizeCallback&);
    void InsertFinalizeCallback(unsigned int, const FinalizeCallback&);
    void DoFinalize();
    
    //Flatbuffers stuff
    void BuildRequest(const std::string&,
                      flatbuffers::FlatBufferBuilder&, flatbuffers::FlatBufferBuilder&);
    inline std::string GetErrorVerbose(const fbs::Status& status){
        switch(status){
            case fbs::Status_OK:
                return "OK";
                
            case fbs::Status_AUTH_ERROR:
                return "Authorization Error";
                
            case fbs::Status_USER_EXIST:
                return "User already exist";
                
            case fbs::Status_REGISTER_INFO_INVALID:
                return "Registaration Error";
                
            case fbs::Status_PAYLOAD_FORMAT_INVALID:
                return "Invalid Payload Format";
                
            default:
                return "Unknown Error";
        }
    }
    
    //Account stuff
    namespace account{
        std::string GetNickName(const std::string& username);
    } //namespace account
    
    //UDP stuff
    int udp_connect(/*IPv4*/const char*, int);
    
    typedef std::function<void(char*,ssize_t)> ResponseHandleFunc;
    void ClientSendAndRead(int socket_fd,
                           flatbuffers::FlatBufferBuilder& builder,
                           const ResponseHandleFunc& callback);
    
}; //namespace utils

#endif