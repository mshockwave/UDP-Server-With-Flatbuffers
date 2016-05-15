#ifndef _CLIENT_CONTEXT_HPP_
#define _CLIENT_CONTEXT_HPP_

#include <string>
#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace context {
    
    extern std::string CurrentTokenStr;
    
    extern int SocketFd;
    
    extern const char PROMPT_CHAR;
    
    extern std::string Username;
    
    namespace post{
        extern long CurrentPid, MaxPid;
        extern long CurrentCid, MaxCid;
    } //namespace post
    
    namespace account {
        
        extern std::vector<std::string> PendingFriends;
        
    } //namespace account
    
    namespace msg{
        
        typedef std::string channel_id_t;
        
        extern channel_id_t CurrentChannelId;
        extern std::vector<channel_id_t> Channels;
        
        //File name -> fd
        extern std::unordered_map<std::string, int> TransferFdMap;
        
    } //namespace msg
    
    inline void PrintDivideLine(){
        int i;
        for(i = 0; i < 30; i++) std::cout << '=';
        std::cout << std::endl;
    }
    
    enum Screen{
        ENTRY = 0,
        STAY = 1,
        QUIT = 2,
        REGISTER = 3,
        LOGIN = 4,
        LOGOUT = 5,
        MAIN = 6,
        
        //Post
        ADD_POST = 7,
        EDIT_POST = 8,
        VIEW_NEXT_POST = 9,
        VIEW_PREV_POST = 10,
        LIKE_POST = 11,
        UNLIKE_POST = 12,
        ADD_COMMENT = 13,
        VIEW_NEXT_COMMENTS = 14,
        VIEW_PREV_COMMENTS = 15,
        REMOVE_POST = 16,
        REMOVE_COMMENT = 17,
        
        //Account
        FRIEND_ENTRY = 18,
        SEARCH_ACCOUNT = 19,
        VIEW_FRIENDS = 20,
        VIEW_PENDING_FRIENDS = 21,
        ADD_FRIEND = 22,
        CONFIRM_PENDING_FRIEND = 23,
        
        //Message
        MSG_ENTRY = 24,
        MSG_CREATE_PRIVATE_CHANNEL = 25,
        MSG_CREATE_GROUP_CHANNEL = 26,
        MSG_VIEW_CHANNELS = 27,
        MSG_MAIN_WINDOW = 28,
        MSG_JOIN_GROUP = 29,
        MSG_JOIN_CHANNEL = 30
    };
    
    typedef std::function<Screen(Screen)> ScreenHandler;
#define SCREEN_HANDLER() \
    [](context::Screen current_screen)->context::Screen
    
    ScreenHandler& GetScreen(Screen scr);
    void AddScreen(Screen scr, const ScreenHandler& handler);
    
    void InitScreens();
    
} //namespace context

#endif