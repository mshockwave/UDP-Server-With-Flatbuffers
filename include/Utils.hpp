#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <string>
#include <vector>
#include <functional>

#include <schemas/packet_generated.h>

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
    
    //UDP stuff
    int udp_connect(/*IPv4*/const char*, int);
    
}; //namespace utils

#endif