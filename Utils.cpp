#include <Utils.hpp>

extern "C"{
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
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
    
    void ClientSendAndRead(int socket_fd,
                           flatbuffers::FlatBufferBuilder& builder,
                           const ResponseHandleFunc& callback){
        ssize_t n_bytes;
        if( (n_bytes = write(socket_fd, builder.GetBufferPointer(), builder.GetSize())) < 0){
            callback(nullptr, n_bytes);
            return;
        }
        
        fd_set fds;
        FD_ZERO(&fds);
        const auto cleanup_callback = [&]{
            FD_CLR(socket_fd, &fds);
        };
        
        typedef struct timeval TimeVal;
        
        int counter = 0;
        
        while((counter++) < RESEND_THRESHOLD_NUM){
            
            FD_SET(socket_fd, &fds);
            
            auto timeout = TimeVal{0};
            timeout.tv_usec = RESEND_TIMEOUT_US;
            
            int nready;
            if( (nready = select(socket_fd + 1, &fds, nullptr, nullptr, &timeout)) < 0 ){
                if(errno == EINTR){
                    continue;
                }else{
                    //Abort
                    int n_err = (errno < 0)? errno : -errno;
                    cleanup_callback();
                    callback(nullptr, n_err);
                    return;
                }
            }else if(nready == 0){
                //Timeout
                //Retry
                continue;
            }else if(FD_ISSET(socket_fd, &fds)){
                //Receive
                char recv_buffer[RECV_BUFFER_SIZE];
                memset(recv_buffer, 0, sizeof(recv_buffer));
                if( (n_bytes = read(socket_fd, recv_buffer, RECV_BUFFER_SIZE)) < 0){
                    callback(nullptr, n_bytes);
                    cleanup_callback();
                    return;
                }
                
                callback(recv_buffer, n_bytes);
                cleanup_callback();
                return;
            }
            
        }
        
        if(counter >= RESEND_THRESHOLD_NUM){
            //Timeout, failed
            cleanup_callback();
            int n_err = (ETIMEDOUT < 0)? ETIMEDOUT : -ETIMEDOUT;
            callback(nullptr, n_err);
        }
    }
    
}; //namespace handlers