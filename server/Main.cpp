
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

#define SERVER_USAGE \
    "Usage: %s <server port>\n"

#ifndef SOCKET_BUFFER_SIZE
#define SOCKET_BUFFER_SIZE  (240 * (1 << 10)) //240KB
#endif

static int sServerSocket = -1;
static fd_set sFdSet;
static bool sTerminate = false;

void sigKillHandle(int sig){
    puts("Closing server socket...");
    if(sServerSocket >= 0){
        FD_CLR(sServerSocket, &sFdSet);
        close(sServerSocket);
        sTerminate = true;
    }
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
    sServerSocket = serverFd;
    int socket_buffer_size = SOCKET_BUFFER_SIZE;
    setsockopt(serverFd, SOL_SOCKET, SO_RCVBUF, &socket_buffer_size, sizeof(socket_buffer_size));
    bind(serverFd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    signal(SIGKILL, sigKillHandle);
    signal(SIGTERM, sigKillHandle);
    signal(SIGINT, sigKillHandle);
    Log::V("Server Main") << "Listening on 0.0.0.0:" << port << "..." << std::endl;
    
    
    FD_ZERO(&sFdSet);
    int max_fd = serverFd + 1;
    
    //Listener loop
    while(!sTerminate){
        FD_SET(serverFd, &sFdSet);
        
        if(select(max_fd, &sFdSet, nullptr, nullptr, nullptr) < 0){
            if(errno == EINTR) continue;
            else{
                perror("Select error: ");
            }
        }else{
            //Check fds
            if(FD_ISSET(serverFd, &sFdSet)){
                /*TODO: Handle request*/
            }
        }
    }
    
    return 0;
}

