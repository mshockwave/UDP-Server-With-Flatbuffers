#ifndef _CLIENT_CONTEXT_HPP_
#define _CLIENT_CONTEXT_HPP_

#include <string>
#include <functional>
#include <iostream>

namespace context {
    
    extern std::string CurrentTokenStr;
    
    extern int SocketFd;
    
    extern const char PROMPT_CHAR;
    
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
        EDIT_POST = 8
    };
    
    typedef std::function<Screen(void)> ScreenHandler;
#define SCREEN_HANDLER() \
    [](void)->context::Screen
    
    ScreenHandler& GetScreen(Screen scr);
    void AddScreen(Screen scr, const ScreenHandler& handler);
    
    void InitScreens();
    
} //namespace context

#endif