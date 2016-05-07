#include <Utils.hpp>

extern "C"{
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
}

//Hide symbol
static std::vector<utils::FinalizeCallback> FinalizeCallbacks;

namespace utils {
    
    void AddFinalizeCallback(const FinalizeCallback& callback){
        FinalizeCallbacks.insert(FinalizeCallbacks.begin(), callback);
    }
    void PushBackFinalizeCallback(const FinalizeCallback& callback){
        FinalizeCallbacks.push_back(callback);
    }
    void InsertFinalizeCallback(unsigned int index, const FinalizeCallback& callback){
        if(index < FinalizeCallbacks.size()){
            FinalizeCallbacks.insert(FinalizeCallbacks.begin() + index, callback);
        }else{
            PushBackFinalizeCallback(callback);
        }
    }
    
    void DoFinalize(){
        std::vector<FinalizeCallback>::iterator it_callback;
        for(it_callback = FinalizeCallbacks.begin();
            it_callback != FinalizeCallbacks.end(); ++it_callback){
            auto callback = *it_callback;
            callback();
        }
    }
    
    void BuildRequest(const std::string &path,
                      flatbuffers::FlatBufferBuilder &builder_raw,
                      flatbuffers::FlatBufferBuilder &builder_resp){
        
        auto payload = builder_raw.CreateVector((int8_t*)builder_resp.GetBufferPointer(),
                                                builder_resp.GetSize());
        auto raw_req = fbs::CreateRequestPacket(builder_raw,
                                                builder_raw.CreateString(path),
                                                payload);
        fbs::FinishRequestPacketBuffer(builder_raw, raw_req);
    }
    
    int udp_connect(const char* address, int port){
        struct sockaddr_in server_addr;
        bzero(&server_addr, sizeof(server_addr));
        
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, address, &server_addr.sin_addr);
        
        int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
        
        if(connect(sockFd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0){
            //Fail
            close(sockFd);
            sockFd = -1;
        }
        
        return sockFd;
    }
    
}; //namespace handlers