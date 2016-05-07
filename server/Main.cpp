
extern "C"{
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
}

#include <Log.hpp>
#include <Utils.hpp>

#include "Types.hpp"
#include "Account.hpp"
#include "Router.hpp"

#include <schemas/types_generated.h>

#define SERVER_USAGE \
    "Usage: %s <server port>\n"

#ifndef SOCKET_BUFFER_SIZE
#define SOCKET_BUFFER_SIZE  (240 * (1 << 10)) //240KB
#endif

static void sigKillHandle(int sig){
    Log::V("Server Main") << "Cleaning up..." << std::endl;
    utils::DoFinalize();
}

int main(int argc, char **argv){
    
    int port = -1;
    if(argc - 1 >= 1){
        port = atoi(argv[1]);
    }
    if(port < 0){
        printf(SERVER_USAGE, argv[0]);
        return 1;
    }
    
    /*Setup server*/
    //Server address
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    
    //Server socket
    int serverFd = socket(AF_INET, SOCK_DGRAM, 0);
    int socket_buffer_size = SOCKET_BUFFER_SIZE;
    setsockopt(serverFd, SOL_SOCKET, SO_RCVBUF, &socket_buffer_size, sizeof(socket_buffer_size));
    bind(serverFd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    signal(SIGKILL, sigKillHandle);
    signal(SIGTERM, sigKillHandle);
    signal(SIGINT, sigKillHandle);
    Log::V("Server Main") << "Listening on 0.0.0.0:" << port << "..." << std::endl;
    
    
    fd_set fds;
    FD_ZERO(&fds);
    int max_fd = serverFd + 1;
    utils::AddFinalizeCallback([&](void)->void{
        //Log::D("") << "Clearing select fds and closing server socket..." << std::endl;
        FD_CLR(serverFd, &fds);
        close(serverFd);
    });
    
    //Initialize routers
    Router root_router, account_router;
    handlers::InitAccountHandlers(account_router);
    
    root_router.Path("/account", account_router);
    
    byte_t recv_buffer[RECV_BUFFER_SIZE];
    
    //Listener loop
    bool terminated = false;
    utils::PushBackFinalizeCallback([&](void)->void{
        //Log::D("") << "Setting terminated flag..." << std::endl;
        terminated = true;
    });
    while(!terminated){
        FD_SET(serverFd, &fds);
        
        if(select(max_fd, &fds, nullptr, nullptr, nullptr) < 0){
            if(errno == EINTR) continue;
            else{
                perror("Select error: ");
            }
        }else{
            //Check fds
            if(FD_ISSET(serverFd, &fds)){

                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                memset(recv_buffer, 0, sizeof(recv_buffer));
                
                ssize_t n_bytes = recvfrom(serverFd,
                                           recv_buffer, RECV_BUFFER_SIZE,
                                           0,
                                           (struct sockaddr*)&client_addr, &client_addr_len);
                if(n_bytes < 0){
                    Log::E("Server Main") << "Error receiving message" << std::endl;
                }else{
                    //Verify request
                    flatbuffers::Verifier verifier(recv_buffer, static_cast<size_t>(n_bytes));
                    if(fbs::VerifyRequestPacketBuffer(verifier)){
                        auto* raw_request = fbs::GetRequestPacket(recv_buffer);
                        
                        //Delegate to router
                        const ResponseWriter resp_writer = [&](const byte_t* buffer, size_t buf_size)->ssize_t{
                            return sendto(serverFd,
                                          buffer, buf_size,
                                          0,
                                          (struct sockaddr*)&client_addr, client_addr_len);
                        };
                        root_router.Process(*raw_request, resp_writer);
                    }else{
                        //TODO: Error verifying
                    }
                }
            }
        }
    }
    
    return 0;
}

