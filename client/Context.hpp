#ifndef _CLIENT_CONTEXT_HPP_
#define _CLIENT_CONTEXT_HPP_

#include <string>
#include <functional>
#include <iostream>

namespace context {
    
    extern std::string CurrentTokenStr;
    
    extern int SocketFd;
    
    extern const char PROMPT_CHAR;
    
    extern std::string Username;
    
    namespace post{
        extern long CurrentPid, MaxPid;
        extern long CurrentCid, MaxCid;
    } //namespace post
    
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
        REMOVE_COMMENT = 17
    };
    
    typedef std::function<Screen(Screen)> ScreenHandler;
#define SCREEN_HANDLER() \
    [](context::Screen current_screen)->context::Screen
    
    ScreenHandler& GetScreen(Screen scr);
    void AddScreen(Screen scr, const ScreenHandler& handler);
    
    void InitScreens();
    
} //namespace context

#endif