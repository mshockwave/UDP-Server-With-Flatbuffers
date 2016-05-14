extern "C"{
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
}

#include <Log.hpp>
#include <Utils.hpp>

#include "Context.hpp"
#include "Account.hpp"
#include "Post.hpp"
#include "Message.hpp"

#define USAGE_FORMAT_STR \
    "Usage: %s <server address> <server port>\n"

int main(int argc, char **argv){
    
    if(argc - 1 < 2){
        printf(USAGE_FORMAT_STR, argv[0]);
        return 1;
    }
    
    int port = -1;
    port = atoi(argv[2]);
    if(port <= 0){
        printf(USAGE_FORMAT_STR, argv[0]);
        return 1;
    }
    
    Log::V("Client Main") << "Connecting to " << argv[1] << ":" << port << std::endl;
    
    int sockFd = utils::udp_connect(argv[1], port);
    if(sockFd < 0){
        Log::E("Client Main") << "Connect error: " << std::endl;
        perror("");
        return 1;
    }
    context::SocketFd = sockFd;
    
    //Init screens
    context::InitScreens();
    account::InitScreens();
    post::InitScreens();
    msg::InitScreens();
    
    auto screen = context::Screen::ENTRY;
    do{
        
        const auto& handler = context::GetScreen(screen);
        auto next_screen = handler(screen);
        if(next_screen != context::Screen::STAY){
            screen = next_screen;
        }
        
    }while(screen != context::Screen::QUIT);
    
    close(sockFd);
    
    return 0;
}

